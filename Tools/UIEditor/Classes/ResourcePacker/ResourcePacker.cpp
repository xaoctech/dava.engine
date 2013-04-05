//
//  ResourcePacker.cpp
//  UIEditor
//
//  Created by Denis Bespalov on 1/10/13.
//  Originaly Created by Vitaliy  Borodovsky on 3/21/10 for class ResourcePackerScreen.
//
//

#include "ResourcePacker.h"
#include <Magick++.h>
#include <magick/MagickCore.h>
#include <magick/property.h>
#include "TexturePacker/DefinitionFile.h"
#include "TexturePacker/TexturePacker.h"
#include "TexturePacker/CommandLineParser.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "Utils/StringFormat.h"

static const FilePath SAVED_FILE_LIST_YAML_FILE("filelist.yaml");

ResourcePacker::ResourcePacker()
{
	isLightmapsPacking = false;
	clearProcessDirectory = false;
}

String ResourcePacker::GetProcessFolderName()
{
	return "$process/";
}

void ResourcePacker::PackResources(const FilePath & inputDir, const FilePath & outputDir)
{
    DVASSERT(inputDir.IsDirectoryPathname() && outputDir.IsDirectoryPathname());
    
	// Do not try to pack empty if pack directories are not set
	if (!inputDir.IsInitalized() || !outputDir.IsInitalized())
		return;
		
	// Create output dir for case it is not created
	FileSystem::Instance()->CreateDirectory(outputDir, true);

	CommandLineParser::Instance()->ClearFlags();

	inputGfxDirectory = inputDir;
	outputGfxDirectory = outputDir;
	excludeDirectory = inputDir + FilePath("../");
	
	StartPacking();
}

void ResourcePacker::StartPacking()
{
	Logger::Debug("Input: %s Output: %s Exclude: %s", inputGfxDirectory.GetAbsolutePathname().c_str(), outputGfxDirectory.GetAbsolutePathname().c_str(), excludeDirectory.GetAbsolutePathname().c_str());
	
	isGfxModified = false;

	gfxDirName = inputGfxDirectory.GetLastDirectoryName();
	std::transform(gfxDirName.begin(), gfxDirName.end(), gfxDirName.begin(), ::tolower);

	FilePath processDirectoryPath = excludeDirectory + FilePath(GetProcessFolderName());
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}

	if (IsMD5ChangedDir(excludeDirectory + FilePath(GetProcessFolderName()), outputGfxDirectory, gfxDirName + ".md5", true))
	{
		if (Core::Instance()->IsConsoleMode())
			printf("[Gfx not available or changed - performing full repack]\n");
		isGfxModified = true;
	
		// Remove whole output directory
		bool result = FileSystem::Instance()->DeleteDirectory(outputGfxDirectory);
		if (result)
		{
			Logger::Debug("Removed output directory: %s", outputGfxDirectory.GetAbsolutePathname().c_str());
		}
		if (!result && Core::Instance()->IsConsoleMode())
		{
			printf("[ERROR: Can't delete directory %s]\n", outputGfxDirectory.GetAbsolutePathname().c_str());
		}
	}

	RecursiveTreeWalk(inputGfxDirectory, outputGfxDirectory);

	// Put latest md5 after convertation
	IsMD5ChangedDir(excludeDirectory + FilePath(GetProcessFolderName()), outputGfxDirectory, gfxDirName + ".md5", true);
}

bool ResourcePacker::SaveFileListToYaml(const FilePath & yamlFilePath)
{
	YamlNode fontsNode(YamlNode::TYPE_MAP);
	MultiMap<String, YamlNode*> &fontsMap = fontsNode.AsMap();
	
    for (ResourcePacker::FILESMAP::const_iterator iter = spriteFiles.begin(); iter != spriteFiles.end(); ++iter)
    {
		YamlNode *fileNode = new YamlNode(YamlNode::TYPE_MAP);
		String fileName = iter->first;
		String fileDate = iter->second;
		// Put file date
		fileNode->Set("date", fileDate);
		// Put file node into map
		fontsMap.insert(std::pair<String, YamlNode*>(fileName, fileNode));
    }
	
	YamlParser* parser = YamlParser::Create();
	
	return parser->SaveToYamlFile(yamlFilePath, &fontsNode, true, File::CREATE | File::WRITE);
}

void ResourcePacker::FillSpriteFilesMap(const FilePath & inputPathName)
{
	// Reset sprites files map
	spriteFiles.clear();

	// Get the list of files inside input directory
	FileList * fileList = new FileList(inputPathName);	
	
	if (fileList)
	{
		// Process the list of files inside input directory and fill psdFile map with values
		for(int i = 0; i < fileList->GetCount(); ++i)
		{
			if(!fileList->IsDirectory(i) && !fileList->IsNavigationDirectory(i))
			{
				String fileName = fileList->GetFilename(i);
				String modDate = File::GetModificationDate(fileList->GetPathname(i));

				// spriteFiles into map file date with a key - file name
				spriteFiles[fileName] = modDate;
			}
		}
	}
}

bool ResourcePacker::CheckSpriteFilesDates(YamlNode *rootNode)
{
	// Check here if we the number of files in yaml file and inside input directory is equal
	// If not - we should launch md5 checksum calculation
	if (rootNode->AsMap().size() != spriteFiles.size())
	{
		return true;
	}
	// Compare modify date of saved files list and modify date of actual files inside input directory
	// If only one file differs or new file found - we should launch md5 checksum calculation
	for (MultiMap<String, YamlNode*>::iterator t = rootNode->AsMap().begin(); t != rootNode->AsMap().end(); ++t)
	{
		YamlNode * fileNode = t->second;
		// Skip empty file node
		if (!fileNode) continue;

		YamlNode * dateNode = fileNode->Get("date");
		// Skip empty date node
		if (!dateNode) continue;
													
		String fileName = fileNode->AsString();
		String fileDate = dateNode->AsString();
		// Look for a new files iside sprite's input folder
		FILESMAP::const_iterator iter = spriteFiles.find(fileName);
		
		// If we have a new sprite - we should launch md5 check process
		if (iter == spriteFiles.end())
		{
			return true;
		}
		
		// Compare saved file modify date and actual file modify date
		String saveFileDate = iter->second;
		if (saveFileDate.compare(fileDate) != 0)
		{
			// If modify date of files differs - we should launch md5 check process
			return true;
		}
	}
	
	return false;
}

bool ResourcePacker::IsModifyDateChagedDir(const FilePath & processDirectoryPath, const FilePath & pathName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	bool md5ChecksumNeeded = false;
	FilePath yamlFilePath = processDirectoryPath + SAVED_FILE_LIST_YAML_FILE;
	
	// Get sprite file names inside input folder and put them into files map
	FillSpriteFilesMap(pathName);
		
	if (spriteFiles.size() > 0)
	{
		// Read existing yaml file with saved file names and their modify date
		YamlParser * parser = YamlParser::Create(yamlFilePath);
			
		if (parser && parser->GetRootNode())
		{
			md5ChecksumNeeded = CheckSpriteFilesDates(parser->GetRootNode());
		}
		else
		{
			Logger::Error("Failed to open yaml file or the file is empty: %s", yamlFilePath.GetAbsolutePathname().c_str());
			md5ChecksumNeeded = true;
		}
		
		// Always update yaml file with actual file list
		SaveFileListToYaml(yamlFilePath);
	}

	// Check here if las modified date correspond to
	if (!md5ChecksumNeeded)
	{		
		return false;
	}

	return IsMD5ChangedDir(processDirectoryPath, pathName, "dir.md5", false);
}

bool ResourcePacker::IsMD5ChangedDir(const FilePath & processDirectoryPath, const FilePath & pathname, const String & name, bool isRecursive)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	FilePath md5FileName = processDirectoryPath + FilePath(name);
    md5FileName.ReplaceExtension(".md5");

	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);


	MD5::ForDirectory(pathname, newMD5Digest, isRecursive);

	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	if (!file)
	{
		isChanged = true;
	}
	else
	{
		int32 bytes = file->Write(newMD5Digest, 16);
		DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	}
	SafeRelease(file);

	// if already changed return without compare
	if (isChanged)
		return true;

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;
	
	return isChanged;
}


bool ResourcePacker::IsMD5ChangedFile(const FilePath & processDirectoryPath, const FilePath & pathname, const FilePath & psdName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	FilePath md5FileName = processDirectoryPath + FilePath(psdName);
    md5FileName.ReplaceExtension(".md5");

	uint8 oldMD5Digest[16];
	uint8 newMD5Digest[16];
	bool isChanged = false;
	File * file = File::Create(md5FileName, File::OPEN | File::READ);
	if (!file)
	{
		isChanged = true;		
	}else
	{
		int32 bytes = file->Read(oldMD5Digest, 16);
		DVASSERT(bytes == 16 && "We should always read 16 bytes from md5 file");
	}
	SafeRelease(file);

		
	MD5::ForFile(pathname, newMD5Digest);
	
	file = File::Create(md5FileName, File::CREATE | File::WRITE);
	if (!file)
	{
		isChanged = true;
	}
	else
	{
		int32 bytes = file->Write(newMD5Digest, 16);
		DVASSERT(bytes == 16 && "16 bytes should be always written for md5 file");
	}
	SafeRelease(file);

	for (int32 k = 0; k < 16; ++k)
		if (oldMD5Digest[k] != newMD5Digest[k])
			isChanged = true;
	
	return isChanged;
}

DefinitionFile * ResourcePacker::ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const FilePath & psdName)
{
    DVASSERT(processDirectoryPath.IsDirectoryPathname());
    
	int32 maxTextureSize = 1024;
	if (CommandLineParser::Instance()->IsFlagSet("--tsize2048"))
	{
		maxTextureSize = 2048;
	}
	
	// TODO: Check CRC32
	Vector<Magick::Image> layers;
	
	FilePath psdNameWithoutExtension = FilePath::CreateWithNewExtension(psdName, "");
	
	try 
	{
		Magick::readImages(&layers, psdPathname.ResolvePathname());
		
		if (layers.size() == 0)
		{
			Logger::Error("Number of layers is too low: %s", psdPathname.GetAbsolutePathname().c_str());
			return 0;
		}
		
		if (layers.size() == 1)
		{
			layers.push_back(layers[0]);
		}
		
		int width = (int)layers[0].columns();
		int height = (int)layers[0].rows();
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];			
			
			currentLayer.crop(Magick::Geometry(width,height, 0, 0));
			currentLayer.magick("PNG");
			FilePath outputFile = processDirectoryPath + psdNameWithoutExtension;
			outputFile += String(Format("%d.png", k - 1));
			currentLayer.write(outputFile.ResolvePathname());
		}
		
		
		DefinitionFile * defFile = new DefinitionFile;
		defFile->filename = processDirectoryPath + String("/") + psdNameWithoutExtension + String(".txt");
		defFile->spriteWidth = width;
		defFile->spriteHeight = height;
		defFile->frameCount = (int)layers.size() -1;
		defFile->frameRects = new Rect2i[defFile->frameCount];
		
		for(int k = 1; k < (int)layers.size(); ++k)
		{
			Magick::Image & currentLayer = layers[k];
			Magick::Geometry bbox = currentLayer.page();
			int xOff = (int)bbox.xOff();
			if (bbox.xNegative())
				xOff = -xOff;
			int yOff = (int)bbox.yOff();
			if (bbox.yNegative())
				yOff = -yOff;
			
			defFile->frameRects[k - 1] = Rect2i(xOff, yOff, (int32)bbox.width(), (int32)bbox.height());
			
			if ((defFile->frameRects[k - 1].dx >= maxTextureSize) || (defFile->frameRects[k - 1].dy >= maxTextureSize))
			{
				
				printf("* WARNING * - frame of %s layer %d is bigger than maxTextureSize(%d) layer exportSize (%d x %d) FORCE REDUCE TO (%d x %d). Bewarned!!! Results not guaranteed!!!\n", psdName.GetAbsolutePathname().c_str(), k - 1, maxTextureSize
					   , defFile->frameRects[k - 1].dx, defFile->frameRects[k - 1].dy, width, height);
				defFile->frameRects[k - 1].dx = width;
				defFile->frameRects[k - 1].dy = height;
			}
				
			
			
			if (CommandLineParser::Instance()->IsFlagSet("--add0pixel"))
			{
				
			}else if (CommandLineParser::Instance()->IsFlagSet("--add1pixel"))
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add2pixel"))
			{
				defFile->frameRects[k - 1].dx+=2;
				defFile->frameRects[k - 1].dy+=2;
			}
			else if (CommandLineParser::Instance()->IsFlagSet("--add4pixel"))
			{
				defFile->frameRects[k - 1].dx+=4;
				defFile->frameRects[k - 1].dy+=4;
			}else 
			{
				defFile->frameRects[k - 1].dx++;
				defFile->frameRects[k - 1].dy++;	
			}
		}
		
		return defFile;
	}
	catch( Magick::Exception &error_ )
    {
		std::cout << "Caught exception: " << error_.what() << " file: "<< psdPathname.GetAbsolutePathname() << std::endl;
		return 0;
    }
	return 0;
}

void ResourcePacker::ProcessFlags(const FilePath & flagsPathname)
{
	File * file = File::Create(flagsPathname, File::READ | File::OPEN);
	if (!file)
	{
		Logger::Error("Failed to open file: %s", flagsPathname.GetAbsolutePathname().c_str());
	}
	char flagsTmpBuffer[4096];
	int flagsSize = 0;
	while(!file->IsEof())
	{
		char c;
		int32 readSize = file->Read(&c, 1);
		if (readSize == 1)
		{
			flagsTmpBuffer[flagsSize++] = c;
		}	
	}
	flagsTmpBuffer[flagsSize++] = 0;
	
	currentFlags = flagsTmpBuffer;
	String flags = flagsTmpBuffer;
	
	const String & delims=" ";
	
	// Skip delims at beginning, find start of first token
	String::size_type lastPos = flags.find_first_not_of(delims, 0);
	// Find next delimiter @ end of token
	String::size_type pos     = flags.find_first_of(delims, lastPos);
	// output vector
	Vector<String> tokens;
	
	while (String::npos != pos || String::npos != lastPos)
	{
		// Found a token, add it to the vector.
		tokens.push_back(flags.substr(lastPos, pos - lastPos));
		// Skip delims.  Note the "not_of". this is beginning of token
		lastPos = flags.find_first_not_of(delims, pos);
		// Find next delimiter at end of token.
		pos     = flags.find_first_of(delims, lastPos);
	}
	
	if (CommandLineParser::Instance()->GetVerbose())
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			Logger::Debug("Token: %s", tokens[k].c_str());
		}

	if (Core::Instance()->IsConsoleMode())
	{
		for (int k = 0; k < (int) tokens.size(); ++k)
		{
			String sub = tokens[k].substr(0, 2);
			if (sub != "--")
				printf("\n[WARNING: flag %s incorrect]\n", tokens[k].c_str());
		}
	}
	
	CommandLineParser::Instance()->SetFlags(tokens);
	
	SafeRelease(file);
}

void ResourcePacker::RecursiveTreeWalk(const FilePath & inputPath, const FilePath & outputPath)
{
    DVASSERT(inputPath.IsDirectoryPathname() && outputPath.IsDirectoryPathname());
    
	uint64 packTime = SystemTimer::Instance()->AbsoluteMS();

	// New $process folder structure
	String dataSourceRelativePath = inputPath.ResolvePathname();
	StringReplace(dataSourceRelativePath, excludeDirectory.ResolvePathname(), String(""));

	FilePath processDirectoryPath = excludeDirectory + FilePath(GetProcessFolderName() + dataSourceRelativePath);
	if (FileSystem::Instance()->CreateDirectory(processDirectoryPath, true) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", processDirectoryPath.c_str());
	}

	if(clearProcessDirectory)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(processDirectoryPath, false);
	}

	//String outputPath = outputPath;
	if (FileSystem::Instance()->CreateDirectory(outputPath) == FileSystem::DIRECTORY_CANT_CREATE)
	{
		//Logger::Error("Can't create directory: %s", outputPath.c_str());
	}
	
	CommandLineParser::Instance()->ClearFlags();
	List<DefinitionFile *> definitionFileList;

	// Find flags and setup them
	FileList * fileList = new FileList(inputPath);
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (!fileList->IsDirectory(fi))
		{
			if (fileList->GetFilename(fi) == "flags.txt")
			{
				ProcessFlags(fileList->GetPathname(fi));
				break;
			}
		}
	}
	
	bool modified = isGfxModified;
	// Process all psd / png files

	if (IsModifyDateChagedDir(processDirectoryPath, inputPath))
	{
		modified = true;
	}

	if (modified)
	{
		FileSystem::Instance()->DeleteDirectoryFiles(outputPath, false);
		
		for (int fi = 0; fi < fileList->GetCount(); ++fi)
		{
			if (!fileList->IsDirectory(fi))
			{
				FilePath fullname = fileList->GetPathname(fi);
                String extension = fullname.GetExtension();
				if (extension == ".psd")
				{
					DefinitionFile * defFile = ProcessPSD(processDirectoryPath, fullname, fileList->GetFilename(fi));
					definitionFileList.push_back(defFile);
				}
				else if(isLightmapsPacking && extension == ".png")
				{
					DefinitionFile * defFile = new DefinitionFile();
					defFile->LoadPNG(fullname, processDirectoryPath);
					definitionFileList.push_back(defFile);
				}
				else if (extension == ".pngdef")
				{
					DefinitionFile * defFile = new DefinitionFile();
					if (defFile->LoadPNGDef(fullname, processDirectoryPath))
					{
						definitionFileList.push_back(defFile);
					}
					else 
					{
						SafeDelete(defFile);
					}
				}
			}
		}

		// 
		if (definitionFileList.size() > 0 && modified)
		{
			TexturePacker packer;
				
			if(isLightmapsPacking)
			{
				packer.UseOnlySquareTextures();
				packer.SetMaxTextureSize(2048);
			}

			if (CommandLineParser::Instance()->IsFlagSet("--split"))
			{
				packer.PackToTexturesSeparate(excludeDirectory.c_str(), outputPathWithSlash.c_str(), definitionFileList);
			}
			else
			{
				packer.PackToTextures(excludeDirectory.c_str(), outputPathWithSlash.c_str(), definitionFileList);
			}
		}
	}	

	packTime = SystemTimer::Instance()->AbsoluteMS() - packTime;

	if (Core::Instance()->IsConsoleMode())
	{
		if (CommandLineParser::Instance()->IsExtendedOutput())
		{
			printf("[%d files packed with flags: %s]\n", (int)definitionFileList.size(), currentFlags.c_str());
		}
	
		String result = "[unchanged]";
		if (modified)
			result = "[REPACKED]";

		printf("[%s - %.2lf secs] - %s\n", inputPath.c_str(), (float64)packTime / 1000.0f, result.c_str());
	}

	
	for (std::list<DefinitionFile*>::iterator it = definitionFileList.begin(); it != definitionFileList.end(); ++it)
	{
		DefinitionFile * file = *it;
		SafeDelete(file);
	}
	definitionFileList.clear();
	
	for (int fi = 0; fi < fileList->GetCount(); ++fi)
	{
		if (fileList->IsDirectory(fi))
		{
			String filename = fileList->GetFilename(fi);
			if ((filename != ".") && (filename != "..") && (filename != "$process") && (filename != ".svn"))
			{
				if ((filename.size() > 0) && (filename[0] != '.'))
					RecursiveTreeWalk(inputPath + String("/") + fileList->GetFilename(fi),
									  outputPath + String("/") + fileList->GetFilename(fi));
			}
		}
	}
	
	SafeRelease(fileList);
}
