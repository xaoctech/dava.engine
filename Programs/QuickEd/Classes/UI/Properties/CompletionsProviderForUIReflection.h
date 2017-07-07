#pragma once

#include "UI/Properties/CompletionsProvider.h"

class PackageBaseNode;

class CompletionsProviderForUIReflection : public CompletionsProvider
{
public:
    CompletionsProviderForUIReflection(const String& propertyName, const String& componentName = String());
    ~CompletionsProviderForUIReflection() override;

    QStringList GetCompletions(AbstractProperty* property) override;

private:
    FastName propertyName;
    const Type* componentType = nullptr;
};
