/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/

#include "VersionInfo.h"
#include "Utils/StringFormat.h"


namespace DAVA
{

    VersionInfo::VersionInfo()
    {
        FillVersionHistory();
        SetCurrentBranch();
    }

    VersionInfo::~VersionInfo()
    {
    }

    void VersionInfo::AddVersion( const VersionInfo::SceneVersion& version )
    {
        DVASSERT(versionMap.find(version.version) == versionMap.end());
        versionMap.insert(VersionMap::value_type(version.version, version));
    }

    const VersionInfo::SceneVersion& VersionInfo::GetCurrentVersion() const
    {
        DVASSERT( !versionMap.empty() );
        return versionMap.rbegin()->second;
    }

    void VersionInfo::FillVersionHistory()
    {
        SceneVersion currentVersion;
        currentVersion.version = 12;
        AddVersion( currentVersion );
    }

    void VersionInfo::SetCurrentBranch()
    {
        DVASSERT( !versionMap.empty() );
        TagsMap& tags = versionMap.rbegin()->second.tags;

        tags.insert( TagsMap::value_type( "water", 1 ) );
        tags.insert( TagsMap::value_type( "sky", 2 ) );
        tags.insert( TagsMap::value_type( "grass", 1 ) );
    }

    VersionInfo::eStatus VersionInfo::TestVersion( const SceneVersion& version ) const
    {
        const SceneVersion& current = GetCurrentVersion();

        // Checking version
        if ( current.version < version.version )
            return INVALID;

        // Checking tags
        const TagsMap& tags = version.tags;
        size_t foundedTags = 0;
        for ( VersionMap::const_iterator itVersion = versionMap.begin(); itVersion != versionMap.end(); itVersion++ )
        {
            //if ( itVersion->first < version.version )
            //    continue;

            const TagsMap& currentTags = itVersion->second.tags;

            // Checking for non compatible tags
            for ( TagsMap::const_iterator itTag = tags.begin(); itTag != tags.end(); itTag++ )
            {
                const String& tagName = itTag->first;
                const uint32 tagVer = itTag->second;

                TagsMap::const_iterator itCurTag = currentTags.find( tagName );
                const bool found = ( itCurTag != currentTags.end() ) && ( itCurTag->second != tagVer );
                if ( found )
                    foundedTags++;
            }
        }

        // Map contains more tags, than are implemented in this build
        if ( foundedTags < tags.size() )
            return INVALID;

        if ( current.version > version.version )
        {
            // TODO: add extra check for usability - if no new tags in `versionMap` are found, then return VALID
            return COMPATIBLE;
        }

        if ( current.tags == version.tags )
            return VALID;

        return COMPATIBLE;
    }

}
