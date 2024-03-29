//
//  RPN_CalcApp.swift
//  RPN Calc
//
//  Created by Eric L. Hernes on 3/11/24.
//

import SwiftUI
import SwiftData

class MyAppDelegate: NSObject, NSApplicationDelegate, ObservableObject {
    func application(
        _ application: NSApplication,
        didRegisterForRemoteNotificationsWithDeviceToken deviceToken: Data
    ) {
        // Record the device token.
    }
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        return true
    }
}

@main
struct RPN_CalcApp: App {
  @NSApplicationDelegateAdaptor private var appDelegate: MyAppDelegate
  var _rpnui : RpnCalcUi = RpnCalcUi()
    var sharedModelContainer: ModelContainer = {
        let schema = Schema([
            Item.self,
        ])
        let modelConfiguration = ModelConfiguration(schema: schema, isStoredInMemoryOnly: false)

        do {
            return try ModelContainer(for: schema, configurations: [modelConfiguration])
        } catch {
            fatalError("Could not create ModelContainer: \(error)")
        }
    }()

    var body: some Scene {
        WindowGroup {
            RpnCalcContentView(ui:_rpnui)
              .padding()
        }
        .windowResizability(.contentSize)
        .modelContainer(sharedModelContainer)
    }
  
}
