//
//  RPN_CalcApp.swift
//  RPN Calc
//
//  Created by Eric L. Hernes on 3/11/24.
//

import SwiftUI
import SwiftData

@main
struct RPN_CalcApp: App {
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
        }
        .modelContainer(sharedModelContainer)
    }
}
