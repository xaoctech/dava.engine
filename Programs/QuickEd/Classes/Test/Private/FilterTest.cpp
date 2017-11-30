#include "Classes/Application/QEGlobal.h"

#include "Classes/Test/Private/TestHelpers.h"
#include "Classes/Test/Private/ProjectSettingsGuard.h"
#include "Classes/Test/Private/MockDocumentsModule.h"

#include "Classes/UI/Find/Filters/FindFilter.h"
#include "Classes/UI/Find/Filters/AcceptsInputFilter.h"
#include "Classes/UI/Find/Filters/CompositeFilter.h"
#include "Classes/UI/Find/Filters/ControlNameFilter.h"
#include "Classes/UI/Find/Filters/HasClassesFilter.h"
#include "Classes/UI/Find/Filters/HasErrorsFilter.h"
#include "Classes/UI/Find/Filters/HasComponentFilter.h"
#include "Classes/UI/Find/Filters/HasErrorsAndWarningsFilter.h"
#include "Classes/UI/Find/Filters/PackageVersionFilter.h"
#include "Classes/UI/Find/Filters/NegationFilter.h"
#include "Classes/UI/Find/Filters/PrototypeUsagesFilter.h"
#include "Classes/UI/Find/Finder/Finder.h"

#include "Classes/Modules/ProjectModule/ProjectModule.h"
#include "Classes/Modules/ProjectModule/ProjectData.h"

#include <TArc/Testing/TArcUnitTests.h>
#include <TArc/Testing/TArcTestClass.h>
#include <TArc/Testing/MockDefine.h>

#include <FileSystem/FileSystem.h>
#include <FileSystem/FilePath.h>
#include <FileSystem/File.h>

#include <UI/UIPackageLoader.h>
#include <UI/UIPackage.h>
#include <UI/UIControlBackground.h>

#include <gmock/gmock.h>

namespace DAVA
{
namespace FilterTestDetails
{
class LocalMockModule;
enum eType
{
    DOCUMENT,
    PROJECT
};
}
#define CREATE_FILTER_FILE_INFO(Type, FilterClassName, CreateDocument, ...)  { \
        FilePath path = projectPath + "/DataSource/UI/" + #FilterClassName + "Test.yaml"; \
        StringStream ss; \
        CreateDocument; \
        RefPtr<File> file(File::Create(path, File::CREATE | File::WRITE)); \
        TEST_VERIFY(file != nullptr); \
        TEST_VERIFY(file->WriteString(ss.str(), false)); \
        FilterInfo t(Type, std::make_shared<FilterClassName>(##__VA_ARGS__), path, QStringList()); \
        filterInfos.push_back(t); \
}

#define CREATE_FILTER_FILE_INFO_1(Type, FilterClassName, CreateDocument, Item1, ...)  { \
        FilePath path = projectPath + "/DataSource/UI/" + #FilterClassName + "Test.yaml"; \
        StringStream ss; \
        CreateDocument; \
        RefPtr<File> file(File::Create(path, File::CREATE | File::WRITE)); \
        TEST_VERIFY(file != nullptr); \
        TEST_VERIFY(file->WriteString(ss.str(), false)); \
        FilterInfo t(Type, std::make_shared<FilterClassName>(##__VA_ARGS__), path, { Item1 }); \
        filterInfos.push_back(t); \
}

#define CREATE_FILTER_FILE_INFO_2(Type, FilterClassName, CreateDocument, Item1, Item2, ...)  { \
        FilePath path = projectPath + "/DataSource/UI/" + #FilterClassName + "Test.yaml"; \
        StringStream ss; \
        CreateDocument; \
        RefPtr<File> file(File::Create(path, File::CREATE | File::WRITE)); \
        TEST_VERIFY(file != nullptr); \
        TEST_VERIFY(file->WriteString(ss.str(), false)); \
        FilterInfo t(Type, std::make_shared<FilterClassName>(##__VA_ARGS__), path, { Item1, Item2 }); \
        filterInfos.push_back(t); \
}

DAVA_TARC_TESTCLASS(FilterTest)
{
    BEGIN_TESTED_MODULES();
    DECLARE_TESTED_MODULE(TestHelpers::ProjectSettingsGuard);
    DECLARE_TESTED_MODULE(TestHelpers::MockDocumentsModule);
    DECLARE_TESTED_MODULE(ProjectModule);
    DECLARE_TESTED_MODULE(FilterTestDetails::LocalMockModule)
    END_TESTED_MODULES();

    DAVA_TEST (MockTest)
    {
    }
};

namespace FilterTestDetails
{
class LocalMockModule : public TArc::ClientModule
{
protected:
    void PostInit() override
    {
        using namespace TArc;
        projectPath = TestHelpers::GetTestPath() + "FilterTest";
        TestHelpers::CreateProjectFolder(projectPath);
        String projectPathStr = projectPath.GetAbsolutePathname();
        InvokeOperation(ProjectModuleTesting::CreateProjectOperation.ID, QString::fromStdString(projectPathStr));

        DataContext* globalContext = GetAccessor()->GetGlobalContext();
        ProjectData* projectData = globalContext->GetData<ProjectData>();
        TEST_VERIFY(projectData != nullptr);
        TEST_VERIFY(projectData->GetUiDirectory().absolute.IsEmpty() == false);
        TEST_VERIFY(projectData->GetProjectDirectory().IsEmpty() == false);

        InitFilterDocuments();

        QStringList allFiles;
        for (FilterInfo filterInfo : filterInfos)
        {
            FilterTestDetails::eType type;
            std::shared_ptr<FindFilter> filter;
            FilePath path;
            QStringList foundItems;
            std::tie(type, filter, path, foundItems) = filterInfo;
            allFiles.push_back(QString::fromStdString(path.GetAbsolutePathname()));
        }
        for (FilterInfo filterInfo : filterInfos)
        {
            FilterTestDetails::eType type;
            std::shared_ptr<FindFilter> filter;
            FilePath path;
            QStringList itemsToFind;
            std::tie(type, filter, path, itemsToFind) = filterInfo;
            QStringList foundItems;
            Finder* finder = new Finder(filter, &projectData->GetPrototypes());
            bool foundAny = false;
            QObject::connect(finder, &Finder::ItemFound, [&foundItems, &foundAny](const FindItem& item) {
                foundAny = true;
                Vector<String> paths = item.GetControlPaths();
                for (String path : paths)
                {
                    foundItems.push_back(QString::fromStdString(path));
                }
            });

            if (type == DOCUMENT)
            {
                finder->Process({ QString::fromStdString(path.GetAbsolutePathname()) });
            }
            else
            {
                finder->Process(allFiles);
            }
            TEST_VERIFY(foundAny);
            TEST_VERIFY(itemsToFind == foundItems);
        }
    }

    using FilterInfo = std::tuple<FilterTestDetails::eType, std::shared_ptr<FindFilter>, FilePath, QStringList>;
    Vector<FilterInfo> filterInfos;
    FilePath projectPath;

    String CreateHeader() const
    {
        StringStream ss;
        ss << "Header:\n"
           << "     version: \""
           << UIPackage::CURRENT_VERSION
           << "\"\n";
        return ss.str();
    }

    void InitFilterDocuments()
    {
        CREATE_FILTER_FILE_INFO_2(DOCUMENT, AcceptsInputFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"1\"\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"2\"\n"
                                     << "    noInput : true\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"3\"\n"
                                     << "    noInput : false\n",
                                  "1", "3");

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, NegationFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"1\"\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"2\"\n"
                                     << "    noInput : true\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"3\"\n"
                                     << "    noInput : false\n",
                                  "2",
                                  std::make_shared<AcceptsInputFilter>());

        CREATE_FILTER_FILE_INFO_2(DOCUMENT, ControlNameFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"name\"\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"NAME\"\n",
                                  "name", "NAME",
                                  "name", false);

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, ControlNameFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"name\"\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"NAME\"\n",
                                  "name",
                                  "name", true);

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, HasComponentFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"1\"\n"
                                     << "    components:\n"
                                     << "        Background: {}\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"2\"\n",
                                  "1",
                                  Type::Instance<UIControlBackground>());

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, HasErrorsAndWarningsFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"1\"\n"
                                     << "    components:\n"
                                     << "        SizePolicy:\n"
                                     << "            horizontalPolicy: \"FixedSize\"\n"
                                     << "        Anchor:\n"
                                     << "            leftAnchorEnabled: true\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"2\"\n"
                                     << "    components:\n"
                                     << "        SizePolicy:\n"
                                     << "            horizontalPolicy: \"FixedSize\"\n"
                                     << "        Anchor:\n"
                                     << "            topAnchorEnabled: true\n",
                                  "1");

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, HasErrorsFilter,
                                  ss << CreateHeader()
                                     << "ImportedPackages:\n"
                                     << "    ~res:/UI/not_existed_one.yaml\n"
                                     << "Controls:\n"
                                     << "-   prototype: \"not_existed_one/some_control\"\n"
                                     << "    name: \"1\"\n",
                                  "1");

        CREATE_FILTER_FILE_INFO(DOCUMENT, PackageVersionFilter,
                                ss << CreateHeader()
                                   << "Prototypes:\n"
                                   << "-   class: \"UIControl\"\n"
                                   << "    name: \"1\"\n"
                                   << "Controls:\n"
                                   << "-   prototype: \"1\"\n"
                                   << "    name: \"1_1\"\n",
                                19, PackageVersionFilter::eCmpType::EQ);

        CREATE_FILTER_FILE_INFO_1(DOCUMENT, CompositeFilter,
                                  ss << CreateHeader()
                                     << "Controls:\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"name\"\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"NAME\"\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"1\"\n"
                                     << "    components:\n"
                                     << "        Background: {}\n"
                                     << "-   class: \"UIControl\"\n"
                                     << "    name: \"name\"\n"
                                     << "    components:\n"
                                     << "        Background: {}\n"
                                     << "-   class : \"UIControl\"\n"
                                     << "    name : \"2\"\n",
                                  "name",
                                  Vector<std::shared_ptr<FindFilter>>{ std::make_shared<ControlNameFilter>("name", true),
                                                                       std::make_shared<HasComponentFilter>(Type::Instance<UIControlBackground>()) });

        CREATE_FILTER_FILE_INFO_2(PROJECT, PrototypeUsagesFilter,
                                  ss << CreateHeader()
                                     << "ImportedPackages:\n"
                                     << "- \"~res:/UI/PackageVersionFilterTest.yaml\"\n"
                                     << "Controls:\n"
                                     << "-   prototype: \"PackageVersionFilterTest/1\"\n"
                                     << "    name: \"1\"\n",
                                  "1_1", "1",
                                  String("~res:/UI/PackageVersionFilterTest.yaml"), FastName("1"));
    }

    DAVA_VIRTUAL_REFLECTION(LocalMockModule, TArc::ClientModule);
};

DAVA_VIRTUAL_REFLECTION_IMPL(LocalMockModule)
{
    ReflectionRegistrator<LocalMockModule>::Begin()
    .ConstructorByPointer()
    .End();
}
} //namespace SSIT
} //namespace DAVA