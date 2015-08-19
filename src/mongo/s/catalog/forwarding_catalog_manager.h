/**
 *    Copyright (C) 2015 MongoDB Inc.
 *
 *    This program is free software: you can redistribute it and/or  modify
 *    it under the terms of the GNU Affero General Public License, version 3,
 *    as published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Affero General Public License for more details.
 *
 *    You should have received a copy of the GNU Affero General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    As a special exception, the copyright holders give permission to link the
 *    code of portions of this program with the OpenSSL library under certain
 *    conditions as described in each individual source file and distribute
 *    linked combinations including the program with the OpenSSL library. You
 *    must comply with the GNU Affero General Public License in all respects for
 *    all of the code used other than as permitted herein. If you modify file(s)
 *    with this exception, you may extend this exception to your version of the
 *    file(s), but you are not obligated to do so. If you do not wish to do so,
 *    delete this exception statement from your version. If you delete this
 *    exception statement from all source files in the program, then also delete
 *    it in the license file.
 */

#pragma once

#include "mongo/s/catalog/catalog_manager.h"
#include "mongo/s/catalog/dist_lock_manager.h"

namespace mongo {

class ConnectionString;
class NamespaceString;
class ServiceContext;
class ShardRegistry;
class VersionType;
struct ReadPreferenceSetting;

/**
 * The ForwardingCatalogManager is an indirection layer that allows for dynamic switching of
 * catalog manager implementations at runtime, to facilitate upgrade.
 */
class ForwardingCatalogManager final : public CatalogManager {
public:
    ForwardingCatalogManager(ServiceContext* service,
                             const ConnectionString& configCS,
                             ShardRegistry* shardRegistry,
                             const std::string& distLockProcessId);

    /**
     * Constructor for use in tests.
     */
    explicit ForwardingCatalogManager(std::unique_ptr<CatalogManager> actual);
    virtual ~ForwardingCatalogManager();

    ConfigServerMode getMode() override;

    Status startup() override;

    void shutDown(bool allowNetworking = true) override;

    Status enableSharding(const std::string& dbName) override;

    Status shardCollection(OperationContext* txn,
                           const std::string& ns,
                           const ShardKeyPattern& fieldsAndOrder,
                           bool unique,
                           const std::vector<BSONObj>& initPoints,
                           const std::set<ShardId>& initShardsIds) override;

    StatusWith<std::string> addShard(OperationContext* txn,
                                     const std::string* shardProposedName,
                                     const ConnectionString& shardConnectionString,
                                     const long long maxSize) override;

    StatusWith<ShardDrainingStatus> removeShard(OperationContext* txn,
                                                const std::string& name) override;

    Status updateDatabase(const std::string& dbName, const DatabaseType& db) override;

    StatusWith<OpTimePair<DatabaseType>> getDatabase(const std::string& dbName) override;

    Status updateCollection(const std::string& collNs, const CollectionType& coll) override;

    StatusWith<OpTimePair<CollectionType>> getCollection(const std::string& collNs) override;

    Status getCollections(const std::string* dbName,
                          std::vector<CollectionType>* collections,
                          repl::OpTime* opTime) override;

    Status dropCollection(OperationContext* txn, const NamespaceString& ns) override;

    Status getDatabasesForShard(const std::string& shardName,
                                std::vector<std::string>* dbs) override;

    Status getChunks(const BSONObj& query,
                     const BSONObj& sort,
                     boost::optional<int> limit,
                     std::vector<ChunkType>* chunks,
                     repl::OpTime* opTime) override;

    Status getTagsForCollection(const std::string& collectionNs,
                                std::vector<TagsType>* tags) override;

    StatusWith<std::string> getTagForChunk(const std::string& collectionNs,
                                           const ChunkType& chunk) override;

    Status getAllShards(std::vector<ShardType>* shards) override;

    bool runUserManagementWriteCommand(const std::string& commandName,
                                       const std::string& dbname,
                                       const BSONObj& cmdObj,
                                       BSONObjBuilder* result) override;

    bool runReadCommand(const std::string& dbname,
                        const BSONObj& cmdObj,
                        BSONObjBuilder* result) override;

    bool runUserManagementReadCommand(const std::string& dbname,
                                      const BSONObj& cmdObj,
                                      BSONObjBuilder* result) override;

    Status applyChunkOpsDeprecated(const BSONArray& updateOps,
                                   const BSONArray& preCondition) override;

    void logAction(const ActionLogType& actionLog) override;

    void logChange(const std::string& clientAddress,
                   const std::string& what,
                   const std::string& ns,
                   const BSONObj& detail) override;

    StatusWith<SettingsType> getGlobalSettings(const std::string& key) override;

    void writeConfigServerDirect(const BatchedCommandRequest& request,
                                 BatchedCommandResponse* response) override;

    Status createDatabase(const std::string& dbName) override;

    DistLockManager* getDistLockManager() override;

    Status checkAndUpgrade(bool checkOnly) override;

    class ScopedDistLock {
        MONGO_DISALLOW_COPYING(ScopedDistLock);

    public:
        explicit ScopedDistLock(DistLockManager::ScopedDistLock&& theLock)
            : _lock(std::move(theLock)) {}
        ~ScopedDistLock() = default;

#if defined(_MSC_VER) && _MSC_VER < 1900
        ScopedDistLock(ScopedDistLock&& other);
        ScopedDistLock& operator=(ScopedDistLock&& other);
#else
        ScopedDistLock(ScopedDistLock&& other) = default;
        ScopedDistLock& operator=(ScopedDistLock&& other) = default;
#endif

        Status checkStatus() {
            return _lock.checkStatus();
        }

    private:
        DistLockManager::ScopedDistLock _lock;
    };

    StatusWith<ScopedDistLock> distLock(
        StringData name,
        StringData whyMessage,
        stdx::chrono::milliseconds waitFor = DistLockManager::kDefaultSingleLockAttemptTimeout,
        stdx::chrono::milliseconds lockTryInterval = DistLockManager::kDefaultLockRetryInterval);

private:
    template <typename Callable>
    auto retry(Callable&& c) -> decltype(std::forward<Callable>(c)());

    std::unique_ptr<CatalogManager> _actual;
};

}  // namespace mongo
