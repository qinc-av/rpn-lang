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
}

struct RpnCalcContentView: View {
  @ObservedObject private var _ui : RpnCalcUi
  
  @Environment(\.modelContext) private var modelContext
  @Query private var items: [Item]
  @State private var commandText: String = ""
  
  private var softKeys : [String:String]

  init(ui:RpnCalcUi!) {
    _ui = ui
    softKeys = [:]
    for ix in (0..<44) {
      let col = (ix/11)+1
      let row = (ix%11)+1
      let k = String(format: "%d.%d", col,row)
      softKeys[k] = "SK:" + k
    }
    _ui._cv = self;
  }
/*
  private func skBinding(for key: String) -> Binding<String> {
    return .init(
      get: { self.softKeys[key, default: ""] },
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
  
  func makeSoftButton(ix:Int) -> some View {
    let col = (ix/11)+1
    let row = (ix%11)+1
    let k = String(format: "%d.%d", col,row)
    let b = Button(action:{}) {
      Text(k)
        .frame(minWidth:60, maxWidth: .infinity)
    }
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
          ForEach(0..<44, id: \.self) { ix in
            self.makeSoftButton(ix:ix)
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

