// @file rsmember.h
/*
 *    Copyright (C) 2010 10gen Inc.
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
 */

/** replica set member */

#pragma once

namespace mongo {


    /*
        RS_STARTUP    serving still starting up, or still trying to initiate the set
        RS_PRIMARY    this server thinks it is primary
        RS_SECONDARY  this server thinks it is a secondary (slave mode)
        RS_RECOVERING recovering/resyncing; after recovery usually auto-transitions to secondary
        RS_FATAL      something bad has occurred and server is not completely offline with regard to the replica set.  fatal error.
        RS_STARTUP2   loaded config, still determining who is primary
    */
    enum MemberState { 
        RS_STARTUP,
        RS_PRIMARY,
        RS_SECONDARY,
        RS_RECOVERING,
        RS_FATAL,
        RS_STARTUP2,
        RS_UNKNOWN, /* remote node not yet reached */
        RS_ARBITER
    };

    /* this is supposed to be just basic information on a member, 
       and copy constructable. */
    class HeartbeatInfo { 
        unsigned _id;
    public:
        HeartbeatInfo() : _id(0xffffffff) { }
        HeartbeatInfo(unsigned id);
        bool up() const { return health > 0; }
        unsigned id() const { return _id; }
        MemberState hbstate;
        double health;
        time_t upSince;
        time_t lastHeartbeat;
        string lastHeartbeatMsg;
        OpTime opTime;
        bool changed(const HeartbeatInfo& old) const;
    };

    inline HeartbeatInfo::HeartbeatInfo(unsigned id) : _id(id) { 
          hbstate = RS_UNKNOWN;
          health = -1.0;
          lastHeartbeat = upSince = 0; 
    }

    inline bool HeartbeatInfo::changed(const HeartbeatInfo& old) const { 
        return health != old.health ||
               hbstate != old.hbstate;
    }

}
