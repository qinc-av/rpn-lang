//
//  Item.swift
//  RPN Calc
//
//  Created by Eric L. Hernes on 3/11/24.
//

import Foundation
import SwiftData

@Model
final class Item {
    var timestamp: Date
    
    init(timestamp: Date) {
        self.timestamp = timestamp
    }
}
