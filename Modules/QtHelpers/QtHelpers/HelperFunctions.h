#pragma once

#include <functional>

class QString;
namespace QtHelpers
{
void ShowInOSFileManager(const QString& path);

void InvokeInAutoreleasePool(std::function<void()> function);

/** copy recursively `srcDir` contents into `dstDir` */
void CopyRecursively(const QString& srcDir, const QString& dstDir);

} // namespace QtHelpers
