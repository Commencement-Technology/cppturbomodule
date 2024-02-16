#import "AppDelegate.h"

#import <React/RCTBundleURLProvider.h>

#pragma mark C++ TurboModules linking Part 1
#import <NativeMyCppModuleAdapter.h>

#pragma mark AppDelegate implementation

@implementation AppDelegate

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
  self.moduleName = @"cppturbomodule";
  // You can add your custom initial props in the dictionary below.
  // They will be passed down to the ViewController used by React Native.
  self.initialProps = @{};

  return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

- (NSURL *)sourceURLForBridge:(RCTBridge *)bridge
{
  return [self getBundleURL];
}

- (NSURL *)getBundleURL
{
#if DEBUG
  return [[RCTBundleURLProvider sharedSettings] jsBundleURLForBundleRoot:@"index"];
#else
  return [[NSBundle mainBundle] URLForResource:@"main" withExtension:@"jsbundle"];
#endif
}

#pragma mark C++ TurboModules linking Part 2
- (std::shared_ptr<facebook::react::TurboModule>)getTurboModule:(const std::string &)name jsInvoker:(std::shared_ptr<facebook::react::CallInvoker>)jsInvoker
{
  if (name == facebook::react::NativeMyCppModuleAdapter::kModuleName) {
    return std::make_shared<facebook::react::NativeMyCppModuleAdapter>(jsInvoker);
  }
  return nullptr;
}

@end
