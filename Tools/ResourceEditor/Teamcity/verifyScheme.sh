# !/bin/bash

DIR_SCH="xcuserdata/$(whoami).xcuserdatad/xcschemes"

if [ -f "../TemplateProjectMacOS.xcodeproj/$DIR_SCH/TemplateProjectMacOS.xcscheme" ]; then
   echo "scheme for MacOS exists"
else
  mkdir -p "../TemplateProjectMacOS.xcodeproj/$DIR_SCH/"
  cp -Rf schemes/MacOS/* "../TemplateProjectMacOS.xcodeproj/$DIR_SCH" 
fi

if [ -f "../TemplateProjectQt.xcodeproj/$DIR_SCH/TemplateProjectQt.xcscheme" ]; then
   echo "scheme for Qt MacOS exists"
else
  mkdir -p "../TemplateProjectQt.xcodeproj/$DIR_SCH/"
  cp -Rf schemes/Qt/* "../TemplateProjectQt.xcodeproj/$DIR_SCH" 
fi