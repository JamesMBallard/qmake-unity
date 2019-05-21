
# QMake-unity

QMake-unity is a standalone tool to speed up the compilation of qmake based C++ projects.


## General concepts

Unity build, jumbo builds and single compilation unit (SCU) are a technique to speedup C++ compilation process.
It consists of grouping/merging several CPP files before compilation.
QMake-unity automates the process of grouping files and may speedup the compilation by 10% to 90%.

It can act at two levels :

- group CPP files before CL is called
- group header files before MOC is called

There are two optimization levels for moc objects :
 
- call MOC on every QObject classes (headers), then group the cpp files generated by moc to build them in a single pass with CL. (`MOC_LVL_1`)
- group every QObject classes (headers) into a single header, then call moc on this file. (`MOC_LVL_2`, even faster)

QMake-unity provides good integration with Qt 5 tooling system (qmake, moc, qt-creator) and with MSBuild.

Here are general articles if you want to understand how it works :
- [Jumbo, A unity build system for Chromium](https://bit.ly/jumbo-blinkon9)
- [The magic of unity builds](http://buffered.io/posts/the-magic-of-unity-builds/)
[Unity builds](http://onqtam.com/programming/2018-07-07-unity-builds/)

## Requirements

You must install Python 3, and add it to the `PATH` environment variable.
You must have a qmake project, built either from QtCreator or command line.

## Installation

**First build**

Copy paste qmake-unity somewhere near your project(s) directory and add the following content at the end of your project file :

```
UNITY_BUILD = 1
UNITY_MOC_MODE = MOC_LVL_0
include(path/to/qmakeUnity/qmake_unity.pri)
```

You're done !
Run qmake and build, you'll probably have to tune your cpp and headers files. (see [code restriction](#code-restrictions) and [troubleshouting](#troubleshouting))
Some cpp/header files are not compatible with unity build, for example headers that rely on inclusion order.
You can remove them from unity by adding the following comment in the source : `//@NO_UNITY`, then call qmake again. 

**Speedup your build**

Once your project compiles fine with `MOC_LVL_0`, you can give a try to `MOC_LVL_1`.
You'll get a few new errors to fix, but also a faster build.

**Speedup your build again** 

Once your project compiles fine with `MOC_LVL_1`, you could give a try to `MOC_LVL_2`.
`MOC_LVL_2` requires your project to use `#pragma once`. 

If you are currently using include guards, you can easily replace them with pragma once, by typing in a terminal :
```
sudo pip3 install guardonce
guard2once.exe -r src
```

## Configuration

You might want to adjust the following variables in your .pro file before including qmake_unity.pri : 

- `UNITY_DIR`
Choose a path where temporary cpp/h/pri files generated by qmake-unity will be stored.
Default value : `BuildPwd/unity/`

Please read `qmake_unity.pri` contents if you want to put move generated/object files out of source.

- `UNITY_STRATEGY`
There are 3 possible strategies to group cpp files together : 
- `incremental` is the default mode. The group size depends on the project's number of cpp files. This option is recommended for big projects.
- `per-processor` generates one CPP per processor. You can set your number of processors in `unity_config.py`.
- `single-compilation-unit` : generated a single CPP file for your entire project.

- `UNITY_MOC_MODE`
`MOC_LVL_0` : don't optimize MOC process.
`MOC_LVL_1` : group the compilation of moc_file1.cpp and moc_file2.cpp
`MOC_LVL_2` : group MOC invokation of file1.h and file2.h, then build the resulting cpp file
Please note that `MOC_LVL_2` requires "#pragma once" to work instead of classic include guards

There are also a few options in `unity_config.py` : 

- `SAFE_MODE`
By default, qmake-unity will be run in "safe mode" which means that it won't delete a file on your disk.
When it should normally delete a temp file, it will just print a debug message.
This protection has been created to avoid deleting wrong files when configuring qmake-unity for the first time.
After checking that the files that qmake-unity tries to delete are in a tmp directory, you should disable "safe mode".

- `LOG_DEBUG`
By default, qmake-unity will print debug logs to help you understand which files are not supported (not grouped).
When your project is ready, you should disable debug logs.

- `NAMESPACE_WHITELIST`
List of namespace names that are allowed appear in a `using namespace XXX` expression in a group
ex : `NAMESPACE_WHITELIST = ["good::namespace"]`

- `NB_PROCESSORS`

- `MOC_GROUPSIZE`
Number of files that will be mocced together (`MOC_LVL_1` or `MOC_LVL_2`)

## Code restrictions

- With unity builds, `using namespace XXX` can trigger errors when building multiple CPP files together.
The most recurrent error is symbol name conflicts among unrelated namespaces.
That is why, by default, a class that contains `using namespace` will not be grouped by qmake-unity (slower, but safe build).
However, you might want to whitelist a few safe namespaces (especially your owns) to get a faster build.
This is right as long as the class/function/static variables names remain unique.
To whitelist namespaces, please edit the `NAMESPACE_WHITELIST` variable in unity_config.py.

- `#include "moc_XXX.cpp"` and `#include "XXX.moc"` are not supported.
You should add the `//@NO_UNITY` comment on top of these files.

- `#pragma once` should be used instead of include guards to enable `MOC_LVL_2` optimization.
If `MOC_LVL_2` is enabled and `#pragma once` is not found in a header, it will not be grouped by qmake-unity (slower, but safe build)

- PIMPL modules are not supported.
However, if your PIMPL modules does not have the `#include "moc_XXX"` directive at the end of the CPP file, you might try to disable this restriction.

## Troubleshouting

**I encounter many build errors with your scripts ; what should I do ?**

Please add your files step by step.
You should start with `UNITY_MOC_MODE = MOC_LVL_0`
You will meet the following error situations that you should fix one by one :
- duplicated include guards (copy paste). I suggest dropping include guards in favor of pragma once, to enjoy the full power of this tool.
- name conflicts on static variables and local functions.
- cpp files declared in the `HEADERS` variable in qmake.

Example of conflict with two unrelated files that now expose their local symbols to each other : 
```
a.cpp
static QString status = "Ok";
b.cpp
QString status(const Screen &screen, const Display &display) { return display.status(screen); }
```

**One macro has a side effect that triggers compile errors**
```
a.h
#define Status(XXX, YYY) XXX+YYY-2

b.cpp (doesn't include a.h)
void displayStatus(const Screen &screen, const Display &display) {}
```
a.cpp may be grouped with b.cpp though they are not related. The macro `Status` has a very short and common name that will match `displayStatus` and replace it with something irrelevant.
Please choose a longer and unique macro name

**Header files that must be included in a certain order ; or that may cause conflicts.**

That is not a good practice, but sometimes things are like that.
For example, some windows SDK headers must be included in a certain order.
In that case, you might remove the header from unity build by adding `//@NO_UNITY` to the including header.

**There is an error related with unity builds, but I don't know which file causes it.**

If you don't know which files causes an error, you can either : 
  - Edit the `tmp/unity/unity_[0123].cpp` files and comment entries until the error disappears. Then you'll know which file raises the problem.
  - Edit your .pro file to include qmake_unity.pro when the `SOURCES` and `HEADERS` variables are just partially filled, then run qmake.

**There is an error on my build server/my colleague's workstation but it builds fine on my computer. How can I fix it ?**

The sources files are probably not be grouped identically on your computer and the other computer.
For example, it happens when you add and remove files from a project.
Procedure to reproduce : 
- build the project locally
- copy and paste the contents of your colleague's `tmp/unity/` directory in you own tmp directory
- build the project without calling qmake (make only) ; the same error should happen.

**The `__FILE__` macro doesn't return the filename anymore, it return a relative path !**

True ! We cannot resolve this as it is part of the compiler behavior, but there is a workaround.
You can still extract the filename at runtime from the file path, using this code : 

```
#define __QFILE_NAME__ unityExtractFileNameFromPath(__FILE__)

QString unityExtractFileNameFromPath(const char * iFilePath)
{
    QString aFile = iFilePath;
    int aIndex = qMax(aFilePath.lastIndexOf("/"), aFilePath.lastIndexOf("\\"));
    if (aIndex > 0) {
        aFile = aFile.right(aFile.length() - (aIndex+1));
    }
    return aFile;
}
```

## Known issues

* `MOC_LVL_2` works only with pragma once; there is no workaround for now

* Visual studio IDE is not supported : the source files are be displayed twice.
Either build in QtCreator or from command line. Visual studio [may be able to support it](https://devblogs.microsoft.com/cppblog/support-for-unity-jumbo-files-in-visual-studio-2017-15-8-experimental/) but it is not implemented yet in qmake-unity.

* When running qmake, qmake-unity logs appear twice. This is because qmake processes every pro file 3 times, one for Makefile, one for Makefile.debug and another one for Makefile.release

* QMake step takes more time when qmake-unity is enabled. This is normal.

## Performances

Here is a benchmark where we fully build a few open source projects from scratch.
It has been run on a Windows 10 laptop powered by an i5 8250u.

### [QupZilla](https://github.com/QupZilla/qupzilla) built with `MSVC`

| Project part | QMake-unity  | Time             |
| ------------ | ------------ | ---------------- |
| lib          | not enabled  | 3m14             |
| lib          | MOC_LVL_1    | 52s (73% faster) |
| lib          | MOC_LVL_2    | 26s (87% faster) |
| plugins      | not enabled  | 1m15             |
| plugins      | MOC_LVL_1    | 27s (64% faster) |
| plugins      | MOC_LVL_2    | 22s (71% faster) |

### [QtXlsxWriter](https://github.com/dbzhang800/QtXlsxWriter) built with `MinGW`
| QMake-unity  | Time             |
| ------------ | ---------------- |
| not enabled  | 33s             |
| MOC_LVL_1    | 13s (61% faster) |
| MOC_LVL_2    | 13s (61% faster) |

### [Qt Game Engine (QGE)](https://github.com/MeLikeyCode/QtGameEngine)
| Compiler | Qmake-unity  | Time             |
| -------- | ------------ | ---------------- |
| MSVC     | not enabled  | 43s              |
| MSVC     | MOC_LVL_1    | 20s (43% faster) |
| MSVC     | MOC_LVL_2    | 15s (65% faster) |
| MinGW    | not enabled  | 1m08s            |
| MinGW    | MOC_LVL_1    | 38s (44% faster) |
| MinGW    | MOC_LVL_2    | 31s (54% faster) |


## What else can I do to speedup my build

**Pure C++ techniques :**
- Forward declare headers (incremental build)
- Write less templates
- Use precompiled headers (PCH)

**Qt :**
- If you build in VisualStudio, install Qt Visual Studio Tools <= 2.2; which now supports parallel calls of moc.

**Improve your build process :**
- Build CPP files and projects in parallel (ex qmake with subdirs, make -j8, vc++ /MP...)
- Distribute your builds (ex distcc)
- Install a compiler cache (ex ccache)
- Build your project on linux instead of windows (big performance difference, even in a VM)

**Tune your system :**
- On windows, whitelist your project directory in the antivirus
- On windows, disable file indexation
- Buy a RAMDisk, more CPU, etc.

**Use better tools :**
- Zapcc is a fast alternative to Clang
- Gold linker replaces ld (linux)
- Replace Make with Ninja (best combo with CMake)
- Add COTIRE to CMake to handle unity builds and precompiled headers
- Install Incredybuild on Windows

**The following links can also help you :**

General advices :
  - https://stackoverflow.com/questions/373142/what-techniques-can-be-used-to-speed-up-c-compilation-times
  - https://medium.com/@Coder_HarryLee/speeding-up-the-build-of-c-and-c-projects-453ce85dd0e1
  - http://kdemonkey.blogspot.com/2013/11/improving-build-times-of-large-qt-apps.html
  - http://developex.com/blog/how-to-make-your-c-qt-project-build-10x-faster-with-4-optimizations/
  - https://lefticus.gitbooks.io/cpp-best-practices/content/02-Use_the_Tools_Available.html
  - https://blogs.msdn.microsoft.com/vcblog/2016/10/26/recommendations-to-speed-c-builds-in-visual-studio/
  - http://www.bitsnbites.eu/faster-c-builds/

Compile cache :
  - https://blog.mozilla.org/ted/2016/11/21/sccache-mozillas-distributed-compiler-cache-now-written-in-rust/

Build instructions of big projects : 
  - https://chromium.googlesource.com/chromium/src/+/master/docs/windows_build_instructions.md#Faster-builds
  - https://wiki.unrealengine.com/Boost_Compile_Times
