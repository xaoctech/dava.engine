/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Ivan "Dizz" Petrochenko
=====================================================================================*/

#include "SceneExporterTool.h"
#include "SceneExporter.h"

#include "../EditorCommandLineParser.h"

using namespace DAVA;


DAVA::String SceneExporterTool::GetCommandLineKey()
{
    return "-sceneexporter";
}

bool SceneExporterTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    
    inFolder = EditorCommandLineParser::GetCommandParam(String("-indir"));
    outFolder = EditorCommandLineParser::GetCommandParam(String("-outdir"));
    if(inFolder.IsEmpty() && outFolder.IsEmpty())
    {
        errors.insert("Incorrect indir or outdir parameter");
        return false;
    }
    
    inFolder.MakeDirectoryPathname();
    outFolder.MakeDirectoryPathname();
    
    format = EditorCommandLineParser::GetCommandParam(String("-format"));
    if(format.empty())
    {
        errors.insert("Format for export is not set");
        return false;
    }
    
    filename = EditorCommandLineParser::GetCommandParam(String("-processfile"));
    foldername = EditorCommandLineParser::GetCommandParam(String("-processdir"));

    if(!filename.empty())
    {
        commandAction = ACTION_EXPORT_FILE;
    }
    else if(!foldername.empty())
    {
        commandAction = ACTION_EXPORT_FOLDER;
    }
    else
    {
        errors.insert("File or folder for export is not set");
        return false;
    }
    
    return true;
}

void SceneExporterTool::Process()
{
    bool needLocalSceneExporter = (SceneExporter::Instance() == NULL);
    if(needLocalSceneExporter)
    {
        new SceneExporter();
    }

    SceneExporter::Instance()->SetOutFolder(outFolder);
    SceneExporter::Instance()->SetInFolder(inFolder);
    SceneExporter::Instance()->SetExportingFormat(format);
    
    if(commandAction == ACTION_EXPORT_FILE)
    {
        SceneExporter::Instance()->ExportFile(filename, errors);
    }
    else if(commandAction == ACTION_EXPORT_FOLDER)
    {
        SceneExporter::Instance()->ExportFolder(foldername, errors);
    }
    
    if(needLocalSceneExporter)
    {
        SceneExporter::Instance()->Release();
    }
}


