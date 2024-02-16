# How to work with C++ TurboModules

Let's assume that there is following C++ code that needs to be used in both Android & iOS apps

Root directory looks like this:
```
├── android
├── common-cpp # <-------------- shared C++ code that will be used in application
├── ios
├── node_modules
├── src
├── .eslintrc.js
├── app.json
├── index.js
├── babel.config.js
├── metro.config.js
├── package.json
├── tsconfig.json
```

**To make things easier**, let's assume that we can use "workspaces" to symlink some local code inside node_modules (e.g. with Yarn v4, Nx, Lerna or any other monorepo solution)

Let's also assume that the project already has "New Architecture" enabled on both platforms ([check how to enable New Architecture in the app](https://reactnative.dev/docs/the-new-architecture/use-app-template#enable-the-new-architecture))

## Glossary

There will be 2 terms used across below steps - "library" & "adapter":

### "library"

By "library", let's refer to a local package/workspace that will:
- follow the structure of any 3rd party NPM RN dependency:
    - it will have following structure
    ```
    ├── android
    ├── common-cpp
    ├── cpp-turbomodule # <--------- "library" folder
    │   ├── android
    │       ├── src
    │           ├── main
    │               ├── java
    │                   ├── com
    │                       ├── mycppturbopackage
    │                           ├── MyCppTurboPackage.(java|kt)
    │               ├── jni
    │                   ├── CMakeLists.txt
    │       ├── build.gradle
    │   ├── cpp
    │       ├── NativeMyCppModuleAdapter.h
    │       ├── NativeMyCppModuleAdapter.cpp
    │       ├── NativeMyCppModuleBindings.h
    │   ├── src
    │       ├── index.ts
    │       ├── NativeMyCppModule.ts
    │   ├── MyCppTurboPackage.podspec
    │   ├── package.json
    ├── ios
    ├── node_modules
    ├── src
    ├── .eslintrc.js
    ├── app.json
    ├── index.js
    ├── babel.config.js
    ├── metro.config.js
    ├── package.json
    ├── tsconfig.json
    ```
    - it will be recognized by RN CLI and then parsed by Codegen
- consist TypeScript Spec necessary to codegenerate C++ bindings
- forward all calls inside C++ TurboModule to shared C++ code that is located in `common-cpp` directory

### "adapter"

By "adapter", let's refer to C++ TurboModule (placed in `cpp-turbomodule/cpp`) that will forward calls to pure C++ classes from `common-cpp` directory

## 1st step: Let's create "library" boilerplate for C++ TurboModule

### `cpp-turbomodule/package.json`

```json
{
  "private": true,
  "name": "cpp-turbomodule",
  "version": "0.0.1",
  "description": "My awesome package",
  "main": "src",
  "module": "src",
  "react-native": "src",
  "source": "src",
  "repository": "<repository-url>",
  "author": "<author>",
  "license": "MIT",
  "homepage": "<homepage-url>",
  "peerDependencies": {
    "react": ">=18",
    "react-native": ">=0.73"
  },
  "codegenConfig": {
    "name": "MyCppTurboPackage",
    "type": "all",
    "jsSrcsDir": "src",
    "android": {
      "javaPackageName": "com.mycppturbopackage"
    }
  }
}

```

The `codegenConfig` field is configuration for Codegen and its values will determine what, where and how will be generated:
- `name` will determine how codegenerated headers will be named
   - for C++ modules it will be responsible for the codegenerated header
      ```cpp
      #if __has_include(<React-Codegen/${name}JSI.h>)
      #include <React-Codegen/${name}JSI.h>
      #elif __has_include("${name}JSI.h")
      #include "${name}JSI.h"
      #endif
      ```
   - for ObjC modules it will be responsible for the following codegenerated header
      ```objc
      #if __has_include(<${name}/${name}.h>)
      #import <${name}/${name}.h>
      #elif __has_include("${name}.h")
      #import "${name}.h"
      #endif
      ```
- `type` is an enum that will determine if Codegen should parse `components`, `modules` or `all` (both `components` & `modules` at the same time)
- `jsSrcsDir` is a path for a directory where TypeScript specs live
- `android.javaPackageName` determines under which package name the Java codegenerated files will be placed

### `cpp-turbomodule/MyCppTurboPackage.podspec`

**To makes things easier**, let's use the value from `codegenConfig.name` to:
- name the podspec file
   - `${name}.podspec`
- set podspec's `name` property
   - `s.name      = "${name}"`

```ruby
require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name            = "MyCppTurboPackage"
  s.version         = package["version"]
  s.summary         = package["description"]
  s.description     = package["description"]
  s.homepage        = package["homepage"]
  s.license         = package["license"]
  s.platforms       = { :ios => "13.0" }
  s.author          = package["author"]
  s.source          = { :git => package["repository"], :tag => "#{s.version}" }

  s.source_files    = [
    "ios/**/*.{h,m,mm}", # ObjC specific code
    "cpp/**/*.{h,cpp}", # C++ adapter code
  ]

  install_modules_dependencies(s)
end

```

Following podspec will link any ObjC specific code and C++ "adapter" code.

### `cpp-turbomodule/android/build.gradle`

**To make things easier**, let's use the value from `codegenConfig.android.javaPackageName` to:
- have the same package name (and source set structure) for "library" android source code
   - set the `namespace` to the value from `codegenConfig.android.javaPackageName`

```groovy
buildscript {
    ext.safeExtGet = {prop, fallback ->
        rootProject.ext.has(prop) ? rootProject.ext.get(prop) : fallback
    }

    repositories {
        google()
        gradlePluginPortal()
    }
    dependencies {
        classpath("com.android.tools.build:gradle:7.2.1")
        classpath("org.jetbrains.kotlin:kotlin-gradle-plugin:1.8.0")
    }
}

apply plugin: 'com.android.library'
apply plugin: 'kotlin-android'
apply plugin: "com.facebook.react"

android {
    compileSdkVersion safeExtGet('compileSdkVersion', 33)

    namespace "com.mycppturbopackage"

    defaultConfig {
        minSdkVersion safeExtGet('minSdkVersion', 21)
        targetSdkVersion safeExtGet('targetSdkVersion', 33)
        buildConfigField "boolean", "IS_NEW_ARCHITECTURE_ENABLED", "true"
    }

    sourceSets {
        main {
            java.srcDirs += ['src/newarch/java', "${project.buildDir}/generated/source/codegen/java"]
        }
    }

    externalNativeBuild {
        cmake {
            path "src/main/jni/CMakeLists.txt"
        }
    }
}

repositories {
    mavenCentral()
    google()
}

dependencies {
    implementation "com.facebook.react:react-android"
}

```

### `cpp-turbomodule/android/src/main/java/com/mycppturbopackage/MyCppTurboPackage.(java|kt)`

The empty instance of `TurboReactPackage` is needed to make C++ TurboModule recognized by RN CLI & Codegen - this is needed to make Codegen automatically create Java & C++ JSI boilerplate

```kotlin
package com.mycppturbopackage

import com.facebook.react.TurboReactPackage
import com.facebook.react.bridge.NativeModule
import com.facebook.react.bridge.ReactApplicationContext
import com.facebook.react.module.annotations.ReactModule
import com.facebook.react.module.model.ReactModuleInfo
import com.facebook.react.module.model.ReactModuleInfoProvider
import com.facebook.react.turbomodule.core.interfaces.TurboModule

class MyCppTurboPackage : TurboReactPackage() {
    override fun getModule(name: String, reactContext: ReactApplicationContext): NativeModule? {
        return null
    }

    override fun getReactModuleInfoProvider(): ReactModuleInfoProvider {
        val moduleList: Array<Class<out NativeModule?>> = arrayOf(
        )
        val reactModuleInfoMap: MutableMap<String, ReactModuleInfo> = HashMap()
        for (moduleClass in moduleList) {
            val reactModule = moduleClass.getAnnotation(ReactModule::class.java) ?: continue
            reactModuleInfoMap[reactModule.name] =
                ReactModuleInfo(
                    reactModule.name,
                    moduleClass.name,
                    true,
                    reactModule.needsEagerInit,
                    reactModule.isCxxModule,
                    TurboModule::class.java.isAssignableFrom(moduleClass)
                )
        }
        return ReactModuleInfoProvider { reactModuleInfoMap }
    }
}

```

```java
package com.mycppturbopackage;

import androidx.annotation.Nullable;

import com.facebook.react.TurboReactPackage;
import com.facebook.react.bridge.NativeModule;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.module.annotations.ReactModule;
import com.facebook.react.module.model.ReactModuleInfo;
import com.facebook.react.module.model.ReactModuleInfoProvider;
import com.facebook.react.turbomodule.core.interfaces.TurboModule;

import java.util.HashMap;
import java.util.Map;

public class MyCppTurboPackage extends TurboReactPackage {
    @Override
    @Nullable
    public NativeModule getModule(String name, ReactApplicationContext reactContext) {
        return null;
    }

    @Override
    public ReactModuleInfoProvider getReactModuleInfoProvider() {
        Class<? extends NativeModule>[] moduleList = new Class[] {
        };
        final Map<String, ReactModuleInfo> reactModuleInfoMap = new HashMap<>();
        for (Class<? extends NativeModule> moduleClass : moduleList) {
            ReactModule reactModule = moduleClass.getAnnotation(ReactModule.class);

            reactModuleInfoMap.put(
                reactModule.name(),
                new ReactModuleInfo(
                    reactModule.name(),
                    moduleClass.getName(),
                    true,
                    reactModule.needsEagerInit(),
                    reactModule.isCxxModule(),
                    TurboModule.class.isAssignableFrom(moduleClass)
                )
            );
        }

        return new ReactModuleInfoProvider() {
            @Override
            public Map<String, ReactModuleInfo> getReactModuleInfos() {
                return reactModuleInfoMap;
            }
        };
    }
}

```

### `cpp-turbomodule/android/src/main/jni/CMakeLists.txt`

**To makes things easier**, let's use the value from `codegenConfig.name` to name the static library for C++ "adapter" code

```bash
cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
        -fexceptions
        -frtti
        -std=c++=20
        -Wall
        -Wpedantic
        -DFOLLY_NO_CONFIG=1
)

# Use `codegenConfig.name` value from library's `package.json`
set(LIBRARY_TARGET_NAME MyCppTurboPackage)

set(LIBRARY_ANDROID_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../..)
set(LIBRARY_COMMON_DIR ${LIBRARY_ANDROID_DIR}/../cpp)

add_library(${LIBRARY_TARGET_NAME} STATIC
        ${LIBRARY_COMMON_DIR}/NativeMyCppModuleAdapter.cpp
)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC ${LIBRARY_COMMON_DIR})

# link C++ TurboModule dependencies
target_link_libraries(${LIBRARY_TARGET_NAME}
        fbjni
        jsi
        react_nativemodule_core
)

# link Codegen headers
target_link_libraries(${LIBRARY_TARGET_NAME} react_codegen_MyCppTurboPackage)

```

### Empty C++ "adapter" files

Create following empty files (those will be filled with code in next steps):
- `cpp-turbomodule/cpp/NativeMyCppModuleAdapter.h`
- `cpp-turbomodule/cpp/NativeMyCppModuleBindings.h`
- `cpp-turbomodule/cpp/NativeMyCppModuleAdapter.cpp`

### Symlink the "library" and shared C++ code in node_modules

To finish boilerplate, let's symlink the "library" package and `common-cpp` directory. Additionaly, let's mark "library" as direct dependency in `package.json` (for this example, yarn v4 is used)

For "library":
- add `"cpp-turbomodule"` to `"workspaces.packages"` array field in `package.json`
- `"cpp-turbomodule": "*",` add this inside `"dependencies"` field in `package.json`
- run `yarn install`

For `common-cpp`:
- add `package.json` with the following content:

   ```json
   {
      "private": true,
      "name": "common-cpp",
      "version": "0.0.1",
      "description": "Shared C++ package",
      "repository": "<repository-url>",
      "author": "<author>",
      "license": "MIT",
      "homepage": "<homepage-url>"
   }
   ```

- add `"common-cpp"` to `"workspaces.packages"` array field in `package.json`
- run `yarn install`

## 2nd step: Let's write Typescript Spec for C++ TurboModule

### `cpp-turbomodule/src/NativeMyCppModule.ts`

```ts
import type { TurboModule } from 'react-native';
import { TurboModuleRegistry } from 'react-native';
import { Int32 } from 'react-native/Libraries/Types/CodegenTypes';

interface Address {
  street: string;
  city: string;
  zipcode: string;
}

interface User {
  id: Int32;
  name: string;
  hasChildren?: boolean;
  address: Address;
}

export interface Spec extends TurboModule {
  getUsers(user: User): Array<User>;
  getUsersAsync(user: User): Promise<Array<User>>;
}

export default TurboModuleRegistry.getEnforcing<Spec>('MyCppModule');

```

- spec declares the TurboModule with name `MyCppModule` and two methods (sync & async)
- each object type should have its TS interface defined
    - the object `User` type is defined as a TS interface and used as an argument and return value in both methods
    - primitve fields in `User` type uses standard TS types or special Codegen types (`Int32` as in TS there's no distinction between int, float or double types)
    - object field in `User` type is defined as a separate TS interface (`Address` type)

### `cpp-turbomodule/src/index.ts`

```ts
export { default as MyCppModule } from './NativeMyCppModule';

```

If everything is prepared correctly, install Pods with `pod install` inside `ios` directory and run following commands to codegenerate specs:
- `./gradlew generateCodegenArtifactsFromSchema` from application's `android` project directory
- `node node_modules/react-native/scripts/generate-codegen-artifacts.js --path . --outputPath ./ios` from application's root directory

The generated headers can be located in following directories:
- [Android] `cpp-turbomodule/android/build/generated/source/codegen`
- [iOS] `ios/build/generated/ios`

For the C++ TurboModule, the `MyCppTurboPackageJSI.h` is what we look for:
- [Android] `cpp-turbomodule/android/build/generated/source/codegen/jni/react/renderer/components/MyCppTurboPackage/MyCppTurboPackageJSI.h`
- [iOS] `ios/build/generated/ios/MyCppTurboPackageJSI.h`

## 3rd step: Link C++ code in iOS environment

After opening application's project in XCode, you should see empty `NativeMyCppModuleAdapter.h`, `NativeMyCppModuleBindings.h` & `NativeMyCppModuleAdapter.cpp` files under `MyCppTurboPackage` pod in XCode.

If the `common-cpp` code is already linked to app's project in XCode in a way that it can be used by `MyCppTurboPackage` Pod, you can skip to 4th step.

Otherwise, let's create 2nd podspec that will track C++ code from `common-cpp`:

### `common-cpp/CommonCppPackage.podspec`

```ruby
require "json"

package = JSON.parse(File.read(File.join(__dir__, "package.json")))

Pod::Spec.new do |s|
  s.name            = "CommonCppPackage"
  s.version         = package["version"]
  s.summary         = package["description"]
  s.description     = package["description"]
  s.homepage        = package["homepage"]
  s.license         = package["license"]
  s.platforms       = { :ios => "13.0" }
  s.author          = package["author"]
  s.source          = { :git => package["repository"], :tag => "#{s.version}" }

  s.source_files    = [
    "**/*.{h,cpp}", # C++ specific code
  ]
end

```

After creating the file, go to app's `Podfile` and add `pod 'CommonCppPackage', :path => "../common-cpp"` above the `config = use_native_modules!` line

Next, install Pods again and include headers inside `MyCppTurboPackage` Pod's C++ "adapter" as following:

```cpp
// when importing header file `Module.h`
#if __has_include(<CommonCppPackage/Module.h>) // this will work in iOS
#include <CommonCppPackage/Module.h>
#elif __has_include("Module.h") // this will work in Android
#include "Module.h"
#endif
```

## 4th step: Link C++ code in Android environment

To see the "library" inside Android Studio, you need to connect it to application's C++/Java codebase

- Go to `android/app/src/main` and create `jni/CMakeLists.txt` and `jni/OnLoad.cpp` files
- in `android/app/src/main/jni/CMakeLists.txt`
    - paste the content of `node_modules/react-native/ReactAndroid/cmake-utils/default-app-setup/CMakeLists.txt`
        ```bash
        # Copyright (c) Meta Platforms, Inc. and affiliates.
        #
        # This source code is licensed under the MIT license found in the
        # LICENSE file in the root directory of this source tree.

        # This CMake file is the default used by apps and is placed inside react-native
        # to encapsulate it from user space (so you won't need to touch C++/Cmake code at all on Android).
        #
        # If you wish to customize it (because you want to manually link a C++ library or pass a custom
        # compilation flag) you can:
        #
        # 1. Copy this CMake file inside the `android/app/src/main/jni` folder of your project
        # 2. Copy the OnLoad.cpp (in this same folder) file inside the same folder as above.
        # 3. Extend your `android/app/build.gradle` as follows
        #
        # android {
        #    // Other config here...
        #    externalNativeBuild {
        #        cmake {
        #            path "src/main/jni/CMakeLists.txt"
        #        }
        #    }
        # }

        cmake_minimum_required(VERSION 3.13)

        # Define the library name here.
        project(appmodules)

        # This file includes all the necessary to let you build your application with the New Architecture.
        include(${REACT_ANDROID_DIR}/cmake-utils/ReactNative-application.cmake)

        ```
    - at the bottom of the file, add C++ "adapter" code
        ```bash
        add_subdirectory(${REACT_ANDROID_DIR}/../../cpp-turbomodule/android/src/main/jni MyCppTurboPackage_build)
        target_link_libraries(${CMAKE_PROJECT_NAME} MyCppTurboPackage)

        ```

- in `android/app/src/main/jni/OnLoad.cpp`
    - paste the content of `node_modules/react-native/ReactAndroid/cmake-utils/default-app-setup/OnLoad.cpp`
        ```cpp
        /*
         * Copyright (c) Meta Platforms, Inc. and affiliates.
         *
         * This source code is licensed under the MIT license found in  the
         * LICENSE file in the root directory of this source tree.
         */

        // This C++ file is part of the default configuration used by apps and is placed
        // inside react-native to encapsulate it from user space (so you won't need to
        // touch C++/Cmake code at all on Android).
        //
        // If you wish to customize it (because you want to manually link a C++ library
        // or pass a custom compilation flag) you can:
        //
        // 1. Copy this CMake file inside the `android/app/src/main/jni` folder of your
        // project
        // 2. Copy the OnLoad.cpp (in this same folder) file inside the same folder as
        // above.
        // 3. Extend your `android/app/build.gradle` as follows
        //
        // android {
        //    // Other config here...
        //    externalNativeBuild {
        //        cmake {
        //            path "src/main/jni/CMakeLists.txt"
        //        }
        //    }
        // }

        #include <DefaultComponentsRegistry.h>
        #include <DefaultTurboModuleManagerDelegate.h>
        #include <fbjni/fbjni.h>
        #include <react/renderer/componentregistry/ComponentDescriptorProviderRegistry.h>
        #include <rncli.h>

        namespace facebook::react {

        void registerComponents(
            std::shared_ptr<const ComponentDescriptorProviderRegistry> registry) {
            // Custom Fabric Components go here. You can register custom
            // components coming from your App or from 3rd party libraries here.
            //
            // providerRegistry->add(concreteComponentDescriptorProvider<
            //        AocViewerComponentDescriptor>());

            // By default we just use the components autolinked by RN CLI
            rncli_registerProviders(registry);
        }

        std::shared_ptr<TurboModule> cxxModuleProvider(
            const std::string& name,
            const std::shared_ptr<CallInvoker>& jsInvoker) {
            // Not implemented yet: provide pure-C++ NativeModules here.
            return nullptr;
        }

        std::shared_ptr<TurboModule> javaModuleProvider(
            const std::string& name,
            const JavaTurboModule::InitParams& params) {
            // Here you can provide your own module provider for TurboModules coming from
            // either your application or from external libraries. The approach to follow
            // is similar to the following (for a library called `samplelibrary`):
            //
            // auto module = samplelibrary_ModuleProvider(moduleName, params);
            // if (module != nullptr) {
            //    return module;
            // }
            // return rncore_ModuleProvider(moduleName, params);

            // By default we just use the module providers autolinked by RN CLI
            return rncli_ModuleProvider(name, params);
        }

        } // namespace facebook::react

        JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void*) {
            return facebook::jni::initialize(vm, [] {
                facebook::react::DefaultTurboModuleManagerDelegate::cxxModuleProvider =
                    &facebook::react::cxxModuleProvider;
                facebook::react::DefaultTurboModuleManagerDelegate::javaModuleProvider =
                    &facebook::react::javaModuleProvider;
                facebook::react::DefaultComponentsRegistry::
                    registerComponentDescriptorsFromEntryPoint =
                        &facebook::react::registerComponents;
            });
        }

        ```

- in `android/app/build.gradle`
    - register app's CMake
        ```groovy
        android {
            ndkVersion rootProject.ext.ndkVersion
            buildToolsVersion rootProject.ext.buildToolsVersion
            compileSdk rootProject.ext.compileSdkVersion

            namespace "com.cppturbomodule"

            // ...

            // Add this
            externalNativeBuild {
                cmake {
                    path "src/main/jni/CMakeLists.txt"
                }
            }
        }
        ```

If shared code from `common-cpp` directory is not linked yet, create `common-cpp/CMakeLists.txt`:

```bash
cmake_minimum_required(VERSION 3.13)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_STANDARD 20)

add_compile_options(
        -fexceptions
        -frtti
        -std=c++=20
        -Wall
        -Wpedantic
        -DFOLLY_NO_CONFIG=1
)

set(LIBRARY_TARGET_NAME CommonCppPackage)

add_library(${LIBRARY_TARGET_NAME} SHARED
        Module.cpp
        sharedlogic/Address.cpp
        sharedlogic/Sdk.cpp
        sharedlogic/User.cpp
)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        sharedlogic
)

```

After that, let's link it with C++ "adapter" module in `cpp-turbomodule/android/src/main/jni/CMakeLists.txt` by adding following lines at the bottom of the file

```bash
# add shared C++ code
add_subdirectory(${LIBRARY_COMMON_DIR}/../../common-cpp common_cpp_package_build)

target_include_directories(${LIBRARY_TARGET_NAME} PUBLIC ${LIBRARY_COMMON_DIR}/../../common-cpp)

target_link_libraries(${LIBRARY_TARGET_NAME} CommonCppPackage)

```

Now, after syncing Gradle in Android Studio, following folders should be visible in file tree under `cpp-turbomodule/cpp` directory:
- `common-cpp [cppturbomodule.cpp-turbomodule.main]`
    - shared C++ code
- `jni`
    - `CMakeLists.txt`
- `cpp [cppturbomodule.cpp-turbomodule.main]`
    - `NativeMyCppModuleAdapter.h` (empty)
    - `NativeMyCppModuleAdapter.cpp` (empty)

## 5th step: Prepare "adapter"

Open "adapter" source files in your IDE of choice (XCode might be a good choice, because it provides auto-completion) and start with header file

### `cpp-turbomodule/cpp/NativeMyCppModuleAdapter.h`

```cpp
#pragma once

#if __has_include(<React-Codegen/MyCppTurboPackageJSI.h>)
#include <React-Codegen/MyCppTurboPackageJSI.h>
#elif __has_include("MyCppTurboPackageJSI.h")
#include "MyCppTurboPackageJSI.h"
#endif

#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#if __has_include(<CommonCppPackage/Module.h>)
#include <CommonCppPackage/Module.h>
#elif __has_include("Module.h")
#include "Module.h"
#endif

#include "NativeMyCppModuleBindings.h"

namespace facebook::react {

class NativeMyCppModuleAdapter : public NativeMyCppModuleCxxSpec<NativeMyCppModuleAdapter> {
public:
    NativeMyCppModuleAdapter(std::shared_ptr<CallInvoker> jsInvoker);

    std::vector<User> getUsers(jsi::Runtime& rt, User user);
    AsyncPromise<std::vector<sharedlogic::User>> getUsersAsync(jsi::Runtime& rt, sharedlogic::User user);
private:
    common::Module instance;
};

}

```

- `MyCppTurboPackageJSI.h` header will consist `NativeMyCppModuleCxxSpec` codegenerated TurboModule spec class
- `Module.h` is a header from `common-cpp` that consists pure C++ class which will be used as the entrypoint for whole `common-cpp` C++ business logic
- the "adapter" class must extend codegenerated TurboModule spec class and declare all the methods from the spec
   - `common-cpp` entrypoint - `common::Module` - is declared as private variable, it will be initialized in ctor and used to forward calls from spec methods to C++ business logic

As you may noticed, the methods declared in the class are using proper C++ structs and classes instead of `jsi::Object` & `jsi::Value` types. This will be done in `NativeMyCppModuleBindings.h` header file, which will be implemented as a next step

### `cpp-turbomodule/cpp/NativeMyCppModuleBindings.h`

```cpp
#pragma once

#if __has_include(<React-Codegen/MyCppTurboPackageJSI.h>)
#include <React-Codegen/MyCppTurboPackageJSI.h>
#elif __has_include("MyCppTurboPackageJSI.h")
#include "MyCppTurboPackageJSI.h"
#endif

#include <optional>
#include <string>

#if __has_include(<CommonCppPackage/Module.h>)
#include <CommonCppPackage/Module.h>
#elif __has_include("Module.h")
#include "Module.h"
#endif

namespace facebook::react {

#pragma mark Custom C++ structs made with struct generators

    using Address = MyCppModuleBaseAddress<std::string, std::string, std::string>;

    template<>
    struct Bridging<Address>
            : MyCppModuleBaseAddressBridging<std::string, std::string, std::string> {
    };

    using User = MyCppModuleBaseUser<int, std::string, std::optional<bool>, Address>;

    template<>
    struct Bridging<User>
            : MyCppModuleBaseUserBridging<int, std::string, std::optional < bool>, Address> {
};

#pragma mark Manually typed custom C++ structs

template<>
struct Bridging<sharedlogic::Address> {
    static sharedlogic::Address fromJs(
            jsi::Runtime &rt,
            const jsi::Object &value,
            const std::shared_ptr <CallInvoker> &jsInvoker) {
        return sharedlogic::Address{
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "street"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "city"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "zipcode"), jsInvoker),
        };
    }

    static jsi::Object toJs(jsi::Runtime &rt, const sharedlogic::Address &value) {
        auto result = facebook::jsi::Object(rt);
        result.setProperty(rt, "street", bridging::toJs(rt, value.street));
        result.setProperty(rt, "city", bridging::toJs(rt, value.city));
        result.setProperty(rt, "zipcode", bridging::toJs(rt, value.zipcode));
        return result;
    }
};

template<>
struct Bridging<sharedlogic::User> {
    static sharedlogic::User fromJs(
            jsi::Runtime &rt,
            const jsi::Object &value,
            const std::shared_ptr <CallInvoker> &jsInvoker) {
        return sharedlogic::User{
                bridging::fromJs<int>(rt, value.getProperty(rt, "id"), jsInvoker),
                bridging::fromJs<std::string>(rt, value.getProperty(rt, "name"), jsInvoker),
                bridging::fromJs<bool>(rt,
                                       value.hasProperty(rt, "hasChildren") ? value.getProperty(rt,
                                                                                                "hasChildren")
                                                                            : false, jsInvoker),
                bridging::fromJs<sharedlogic::Address>(rt, value.getProperty(rt, "address"),
                                                       jsInvoker),
        };
    }

    static jsi::Object toJs(jsi::Runtime &rt, const sharedlogic::User &value) {
        auto result = facebook::jsi::Object(rt);
        result.setProperty(rt, "id", bridging::toJs(rt, value.id));
        result.setProperty(rt, "name", bridging::toJs(rt, value.name));
        result.setProperty(rt, "hasChildren", bridging::toJs(rt, value.hasChildren));
        result.setProperty(rt, "address", bridging::toJs(rt, value.address));
        return result;
    }
};

}

```

The content of that file showcases what is already documented in RN's docs - ([check Custom Structs section](https://reactnative.dev/docs/the-new-architecture/cxx-custom-types#custom-structs))

- The struct generator approach might be useful in short-term when the object passed from JS layer doesn't need to be used in the same shape when interacting with shared C++ code - otherwise you'd need to parse such struct to any shape supported by shared C++ code
    - `MyCppModuleBaseAddress`, `MyCppModuleBaseAddressBridging`, `MyCppModuleBaseUser`, `MyCppModuleBaseUserBridging` are struct generators imported from `MyCppTurboPackageJSI.h` header
- The manually typed struct approach might be useful in long-term when multiple methods in "adapter" need to use the classes and struct from shared C++ code without additional overhead of converting from/to Codegen'ed structs each time "adapter" interacts with shared C++ code

Now, let's go to implementation file where all forwarding and parsing will take place.

### `cpp-turbomodule/cpp/NativeMyCppModuleAdapter.cpp`

```cpp
#include "NativeMyCppModuleAdapter.h"

namespace facebook::react {

#pragma mark Class implementation

NativeMyCppModuleAdapter::NativeMyCppModuleAdapter(std::shared_ptr<CallInvoker> jsInvoker)
    : NativeMyCppModuleCxxSpec(std::move(jsInvoker)), instance() {}

std::vector<User> NativeMyCppModuleAdapter::getUsers(jsi::Runtime& rt, User user) {
    auto sharedUserArg = sharedlogic::User{
        user.id,
        user.name,
        user.hasChildren.has_value() && user.hasChildren.value(),
        sharedlogic::Address{
            user.address.street,
            user.address.city,
            user.address.zipcode
        }
    };
    auto sharedUsers = instance.getUsers(sharedUserArg);
    std::vector<User> array = {};
    for (auto sharedUser : sharedUsers) {
        array.push_back(User{sharedUser.id, sharedUser.name, sharedUser.hasChildren, Address{sharedUser.address.street, sharedUser.address.city, sharedUser.address.zipcode}});
    }
    return array;
}

AsyncPromise<std::vector<sharedlogic::User>> NativeMyCppModuleAdapter::getUsersAsync(jsi::Runtime& rt, sharedlogic::User user) {
    auto promise = AsyncPromise<std::vector<sharedlogic::User>>(rt, jsInvoker_);
    auto sharedUsers = instance.getUsers(user);
    promise.resolve(sharedUsers);
    return promise;
}

}

```

In `getUsers` method which uses "struct generator" approach, the arguments need to be parsed from Codegen'ed structs to classes provided by `common-cpp` C++ code. And the result from `common-cpp` shared code needs to be parsed back to Codegen'ed structs.

In `getUsersAsync` method which uses "manually typed structs" approach, you can see that the body of method only calls the shared C++ code and returns the promise value - no data marshalling.

## 6th step: Manually register C++ TurboModule on both platforms

As of now, RN CLI does not support autolink of C++ TurboModule

It means the C++ "adapter" instance needs to be created and registered manually

For iOS, go to app's `AppDelegate.mm` and do the following:
- import C++ "adapter" header file - `#import <NativeMyCppModuleAdapter.h>`
- create `getTurboModule:jsInvoker:` method inside `AppDelegate` implementation, where C++ adapter instance will be registered
    ```objc
    - (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:(const std::string &)name 
                                                          jsInvoker:(std::shared_ptr<facebook::react::CallInvoker>)jsInvoker
    {
        if (name == facebook::react::NativeMyCppModuleAdapter::kModuleName) {
            return std::make_shared<facebook::react::NativeMyCppModuleAdapter>(jsInvoker);
        }
        return nullptr;
    }
    ```

For Android, go to `android/app/src/main/jni/OnLoad.cpp` and do the following:
- import C++ "adapter" header file - `#include <NativeMyCppModuleAdapter.h>`
- inside of `cxxModuleProvider` function register the C++ "adapter" instance
    ```cpp
    std::shared_ptr<TurboModule> cxxModuleProvider(
        const std::string& name,
        const std::shared_ptr<CallInvoker>& jsInvoker) {
        if (name == facebook::react::NativeMyCppModuleAdapter::kModuleName) {
            return std::make_shared<facebook::react::NativeMyCppModuleAdapter>(jsInvoker);
        }
        return nullptr;
    }
    ```

## 7th step: Check if everything works

Use the module in JS code (e.g. `App.tsx`) and run the app on both platforms:

```tsx
import * as React from 'react';
import { SafeAreaView, ScrollView, StyleSheet, Text } from 'react-native';
import { MyCppModule } from 'cpp-turbomodule';

const user1 = {
  address: { city: 'London', street: '47 West Street', zipcode: 'N97 6QJ' },
  id: 1,
  name: 'John Doe',
};
const user2 = {
  address: { city: 'London', street: '97 York Road', zipcode: 'NW91 5RU' },
  id: 1,
  name: 'Jane Doe',
  hasChildren: true,
};

const App = () => {
  const [syncUsers, setSyncUsers] = React.useState<any[]>([]);
  const [asyncUsers, setAsyncUsers] = React.useState<any[]>([]);
  React.useEffect(() => {
    const users = MyCppModule.getUsers(user1);
    setSyncUsers(users);
    MyCppModule.getUsersAsync(user2).then(setAsyncUsers);
  }, []);

  return (
    <SafeAreaView>
      <ScrollView style={styles.container}>
        <Text style={styles.sectionDescription}>
          For user {JSON.stringify(user1, null, 2)} we have following relatives{' '}
          {JSON.stringify(syncUsers, null, 2)}
        </Text>
        <Text style={styles.sectionDescription}>
          For user {JSON.stringify(user2, null, 2)} we have following relatives{' '}
          {JSON.stringify(asyncUsers, null, 2)}
        </Text>
      </ScrollView>
    </SafeAreaView>
  );
};

const styles = StyleSheet.create({
  container: {
    backgroundColor: 'white',
    padding: 10,
  },
  sectionDescription: {
    color: 'black',
    fontSize: 18,
    fontWeight: '400',
    marginTop: 8,
  },
});

export default App;

```
