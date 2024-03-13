//
//  ContentView.swift
//  RPN Calc
//
//  Created by Eric L. Hernes on 3/11/24.
//

import SwiftUI
import SwiftData

let hbgf : CGFloat = 60
let hbgs : CGFloat = 5

let sbGI = GridItem(.fixed(25), spacing:3, alignment:.leading)

@objc class RpnCalcUi : NSObject, ObservableObject {
  @Published var stackDisplay: String = ""
  @Published var status: String = ""
  
  var _cv : RpnCalcContentView!;
  var _rpn : RpnCalcController?;
  
  init(withRpn:Bool = true) {
    super.init()
    if (withRpn) {
      _rpn = RpnCalcController(ui:self)
    }
    _cv = nil
  }
  
  @objc func commandFinished(result:Int) -> Void {
    self.stackDisplay = _rpn?.stackAsString() ?? "no rpn"
    self.status = _rpn?.status() ?? "no rpn"
  }
  @objc func assignButton(col:Int, row:Int, rpnword:String, label:String) {
    let k = String(format: "%d.%d", col,row)
    _cv.changeSoftkey(k: k, l:label, w:rpnword)
  }

  @objc func clearAssignedButtons() {
    _cv.softKeys.keys.forEach({ k in
      _cv.changeSoftkey(k: k, l: "", w:"")
    })
  }
}

class RpnWordLabel : ObservableObject, CustomStringConvertible {
  init(label:String="", word:String="") {
    self.label = label
    self.word = word
  }
  var label:String
  var word:String
  var description : String {
    return "(l \(label))(w \(word))"
  }
}

struct RpnWordButton: View {
    var body: some View {
        Button(action: {
          rccv.sendCommandAndEval(rpnword.word)
        }) {
          Text(rpnword.label)
            .frame(minWidth:60, maxWidth: .infinity)
        }
    }
    @ObservedObject var rpnword: RpnWordLabel
    let rccv:RpnCalcContentView
}

struct RpnCalcContentView: View {
  @ObservedObject private var _ui : RpnCalcUi
  
  @Environment(\.modelContext) private var modelContext
  @Query private var items: [Item]
  @State private var commandText: String = ""
  
  var softKeys : [String:RpnWordLabel] = [
    "1.1" : RpnWordLabel(label:"1.1", word:"0"),
    "1.2" : RpnWordLabel(label:"1.2", word:"1"),
    "1.3" : RpnWordLabel(label:"1.3", word:"2"),
    "1.4" : RpnWordLabel(label:"1.4", word:"3"),
    "1.5" : RpnWordLabel(label:"1.5", word:"4"),
    "1.6" : RpnWordLabel(label:"1.6", word:"5"),
    "1.7" : RpnWordLabel(label:"1.7", word:"6"),
    "1.8" : RpnWordLabel(label:"1.8", word:"7"),
    "1.9" : RpnWordLabel(label:"1.9", word:"8"),
    "1.10" : RpnWordLabel(label:"1.10", word:"9"),
    "1.11" : RpnWordLabel(label:"1.11", word:"10"),
    "2.1" : RpnWordLabel(label:"2.1", word:"11"),
    "2.2" : RpnWordLabel(label:"2.2", word:"12"),
    "2.3" : RpnWordLabel(label:"2.3", word:"13"),
    "2.4" : RpnWordLabel(label:"2.4", word:"14"),
    "2.5" : RpnWordLabel(label:"2.5", word:"15"),
    "2.6" : RpnWordLabel(label:"2.6", word:"16"),
    "2.7" : RpnWordLabel(label:"2.7", word:"17"),
    "2.8" : RpnWordLabel(label:"2.8", word:"18"),
    "2.9" : RpnWordLabel(label:"2.9", word:"19"),
    "2.10" : RpnWordLabel(label:"2.10", word:"20"),
    "2.11" : RpnWordLabel(label:"2.11", word:"21"),
    "3.1" : RpnWordLabel(label:"3.1", word:"22"),
    "3.2" : RpnWordLabel(label:"3.2", word:"23"),
    "3.3" : RpnWordLabel(label:"3.3", word:"24"),
    "3.4" : RpnWordLabel(label:"3.4", word:"25"),
    "3.5" : RpnWordLabel(label:"3.5", word:"26"),
    "3.6" : RpnWordLabel(label:"3.6", word:"27"),
    "3.7" : RpnWordLabel(label:"3.7", word:"28"),
    "3.8" : RpnWordLabel(label:"3.8", word:"29"),
    "3.9" : RpnWordLabel(label:"3.9", word:"30"),
    "3.10" : RpnWordLabel(label:"3.10", word:"31"),
    "3.11" : RpnWordLabel(label:"3.11", word:"32"),
    "4.1" : RpnWordLabel(label:"4.1", word:"33"),
    "4.2" : RpnWordLabel(label:"4.2", word:"34"),
    "4.3" : RpnWordLabel(label:"4.3", word:"35"),
    "4.4" : RpnWordLabel(label:"4.4", word:"36"),
    "4.5" : RpnWordLabel(label:"4.5", word:"37"),
    "4.6" : RpnWordLabel(label:"4.6", word:"38"),
    "4.7" : RpnWordLabel(label:"4.7", word:"39"),
    "4.8" : RpnWordLabel(label:"4.8", word:"40"),
    "4.9" : RpnWordLabel(label:"4.9", word:"41"),
    "4.10" : RpnWordLabel(label:"4.10", word:"42"),
    "4.11" : RpnWordLabel(label:"4.11", word:"43"),
    ]

  init(ui:RpnCalcUi!) {
    _ui = ui

    if (true) {
      for ix in (0..<44) {
        let col = (ix/11)+1
        let row = (ix%11)+1
        let k = String(format: "%d.%d", col,row)
//        softButtons[k]  = //
//        print("\"\(k)\" : RpnWordLabel(name:\"\(k)\", word:\"\(ix)\"),")
      }
    }
    _ui._cv = self;
  }

  mutating func changeSoftkey(k:String, l:String, w:String) {
    softKeys[k]?.word = w
    softKeys[k]?.label = l
  }
  /*
  private func skBinding(for key: String) -> Binding<RpnWordLabel> {
    return .init(
      get: { self.softKeys[key, default: RpnWordLabel()] },
      set: { self.softKeys[key] = $0 })
  }
   */
  let hbCols = [
    GridItem(.fixed(hbgf), spacing:hbgs, alignment:.leading),
    GridItem(.fixed(hbgf), spacing:hbgs, alignment:.leading),
    GridItem(.fixed(hbgf), spacing:hbgs, alignment:.leading),
    GridItem(.fixed(hbgf), spacing:hbgs, alignment:.leading),
    GridItem(.fixed(hbgf * 1.5), spacing:hbgs, alignment:.leading),
    GridItem(.fixed(hbgf * 1.5), spacing:hbgs, alignment:.leading)
  ]
  let sbRows = [sbGI, sbGI, sbGI, sbGI, sbGI, sbGI, sbGI, sbGI, sbGI, sbGI, sbGI]
  
  let stackFont = Font
    .system(size: 12)
    .monospacedDigit()
  func addToCommand(val:String) {
    self.commandText += val;
  }
  
  func makeHardButton(_ l:String, _ action:(()->Void)?=nil, xw:CGFloat = 1) -> some View {
    let ww : CGFloat = (40*xw+(5*(xw-1)))
    let b = Button(action:action ?? {self.addToCommand(val:l)}) {
      Text(l)
        .frame(width: ww)
    }
      .frame(width: ww)
      .buttonStyle(.bordered)
    return b
  }
  
  func makeSoftButton(key:String) -> some View {
    let b = RpnWordButton(rpnword:softKeys[key] ?? RpnWordLabel(), rccv:self)
      .frame(minWidth:60)
      .buttonStyle(.bordered)
    return b;
  }
  
  func sendCommandAndEval(_ cmd:String) {
    if (commandText != "") {
      _ui._rpn?.eval(commandText)
      commandText = ""
    }
    _ui._rpn?.eval(cmd)
  }

  func onEnterKey() {
    if (commandText == "") {
      _ui._rpn?.eval("DUP");
    } else {
      _ui._rpn?.eval(commandText);
      commandText = "";
    }
  }

  func onBackDeleteKey() {
    if (commandText == "") {
      _ui._rpn?.eval("DROP")
    } else {
      commandText = String(commandText.dropLast())
    }
  }

  func commandHistoryPrevious() {
  }

  func commandHistoryNext() {
  }

  public var body: some View {
    VStack(alignment: .center) {
      HStack(alignment: .bottom) {
        VStack {
          Text("\(_ui.stackDisplay)")
            .font(stackFont)
            .multilineTextAlignment(.trailing)
            .frame(width:400, height: 130, alignment: .bottomTrailing)
            .padding()
            .overlay(
              Rectangle()
                .stroke(Color.black, lineWidth: 1)
            )
          
          LazyVGrid(columns: hbCols) {
            
            TextField("command entry", text: $commandText)
              .frame(width: 310, alignment:.leading)
              .multilineTextAlignment(.trailing)
            
            Color.clear
            Color.clear
            Color.clear
            Color.clear
            
            self.makeHardButton("⌫", {
              onBackDeleteKey()
            }, xw:1.5)
            self.makeHardButton("7")
            self.makeHardButton("8")
            self.makeHardButton("9")
            self.makeHardButton("/", {
              sendCommandAndEval("/")
            })
            self.makeHardButton("Clear", {
              sendCommandAndEval("CLEAR")
            }, xw:1.5)
            self.makeHardButton("MATH", {
              _ui._rpn?.eval("math-keys")
            }, xw:1.5)
            
            self.makeHardButton("4")
            self.makeHardButton("5")
            self.makeHardButton("6")
            self.makeHardButton("*", {
              sendCommandAndEval("*")
            })
            self.makeHardButton("OVER", {
              sendCommandAndEval("OVER")
            }, xw:1.5)
            self.makeHardButton("STACK", {
              _ui._rpn?.eval("stack-keys")
            }, xw:1.5)
            
            self.makeHardButton("1")
            self.makeHardButton("2")
            self.makeHardButton("3")
            self.makeHardButton("-", {
              sendCommandAndEval("-")
            })
            self.makeHardButton("SWAP", {
              sendCommandAndEval("SWAP")
            }, xw:1.5)
            self.makeHardButton("TYPE", {
              _ui._rpn?.eval("type-keys")
            }, xw:1.5)
            
            self.makeHardButton(".")
            self.makeHardButton("0")
            self.makeHardButton("±", {
              sendCommandAndEval("NEG")
            })
            self.makeHardButton("+", {
              sendCommandAndEval("-")
            })
            self.makeHardButton("ENTER",{
              onEnterKey()
            }, xw:2.5)
          }
          .frame(width: 400.0)
        }
        
        LazyHGrid(rows: sbRows) {
          ForEach(0..<44) { ix in
          let col = (ix/11)+1
          let row = (ix%11)+1
          let k = String(format: "%d.%d", col,row)
            self.makeSoftButton(key:k)
          }
        }
      }
      Text("\(_ui.status)")
        .multilineTextAlignment(.leading)
        .frame(width:500, height:25, alignment:.leading)
        .fixedSize(horizontal:false,vertical:true)
    }
    .frame(alignment:.center)
    .onKeyPress { keypress in
      switch (keypress.key) {
      case .upArrow:
        commandHistoryPrevious()
      case .downArrow:
        commandHistoryNext()
      case .return:
        onEnterKey();
      case .tab:
        sendCommandAndEval("SWAP")
      case .delete:
        onBackDeleteKey()
      default:
        return .ignored
      }
      return .handled
    }

  }
  
  
  var originalBody: some View {
    
    NavigationSplitView {
      List {
        ForEach(items) { item in
          NavigationLink {
            Text("Item at \(item.timestamp, format: Date.FormatStyle(date: .numeric, time: .standard))")
          } label: {
            Text(item.timestamp, format: Date.FormatStyle(date: .numeric, time: .standard))
          }
        }
        .onDelete(perform: deleteItems)
      }
#if os(macOS)
      .navigationSplitViewColumnWidth(min: 180, ideal: 200)
#endif
      .toolbar {
#if os(iOS)
        ToolbarItem(placement: .navigationBarTrailing) {
          EditButton()
        }
#endif
        ToolbarItem {
          Button(action: addItem) {
            Label("Add Item", systemImage: "plus")
          }
        }
      }
    } detail: {
      Text("Select an item")
    }
    
  }
  
  
  private func addItem() {
    withAnimation {
      let newItem = Item(timestamp: Date())
      modelContext.insert(newItem)
    }
  }
  
  
  private func deleteItems(offsets: IndexSet) {
    withAnimation {
      for index in offsets {
        modelContext.delete(items[index])
      }
    }
  }
  
}

#Preview {
  var rpn = RpnCalcUi(withRpn:false)
  return RpnCalcContentView(ui:rpn)
    .modelContainer(for: Item.self, inMemory: true)
}

