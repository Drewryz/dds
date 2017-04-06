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

#include <cstdint>
#include <iostream>
#include <limits>

#include "mongo/bson/timestamp.h"

namespace mongo {

class SnapshotName {
public:
    explicit SnapshotName(uint64_t value) : _value(value) {}
    explicit SnapshotName(Timestamp timestamp) : _value(timestamp.asULL()) {}
    SnapshotName() : _value(0) {}

    /**
     * Returns a SnapshotName guaranteed to compare < all names of actual snapshots.
     */
    static SnapshotName min() {
        return SnapshotName(std::numeric_limits<uint64_t>::min());
    }

    /**
     * Returns a SnapshotName guaranteed to compare > all names of actual snapshots.
     */
    static SnapshotName max() {
        return SnapshotName(std::numeric_limits<uint64_t>::max());
    }

    /**
     * Returns an unsigned number that compares with the same ordering as the SnapshotName.
     */
    uint64_t asU64() const {
        return _value;
    }

    std::string toString() const {
        std::stringstream str;
        str << std::hex << _value;
        return str.str();
    }

    bool operator==(const SnapshotName& rhs) const {
        return _value == rhs._value;
    }
    bool operator!=(const SnapshotName& rhs) const {
        return _value != rhs._value;
    }
    bool operator<(const SnapshotName& rhs) const {
        return _value < rhs._value;
    }
    bool operator<=(const SnapshotName& rhs) const {
        return _value <= rhs._value;
    }
    bool operator>(const SnapshotName& rhs) const {
        return _value > rhs._value;
    }
    bool operator>=(const SnapshotName& rhs) const {
        return _value >= rhs._value;
    }

    friend std::ostream& operator<<(std::ostream& out, const SnapshotName& snapshotName) {
        return out << snapshotName.toString();
    }

private:
    uint64_t _value;
};
}
