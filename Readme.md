<!-- # SmartGenius
1. npm 版本 >=3.0  下载网址:  https://nodejs.org/en/
2. electron 版本 1.4.10 下载网址: https://github.com/electron/electron/releases/tag/v1.4.10
3. Visual Studio Code 下载网址: https://code.visualstudio.com/
4. cd 进入到项目的目录中
5. 运行 npm install 
6. 运行 npm install -g gulp
7. 运行 npm install -g node-gyp
8. 第一次执行请用 gulp rebuild 
9.修改tasks.json文件，设置Visual Studio Code 快捷编译热键(Win:Shift+Ctrl+B Mac:Shift+Command+B)执行命令
   将 taskName =‘default’ 中的 isBuildCommand = true , taskName =‘rebuild’ 中的 isBuildCommand = false  按编译热键后则 default命令 反之则执行rebuild命令
10.修改launch.json文件，设置electron 启动App(热键 Win:F5 Mac:Fn+F5 )
   win 配置如下：
     {
        "version": "0.2.0",
        "configurations": [

            {
                "name": "Launch Electron App",
                "type": "node",
                "program": "${workspaceRoot}/App/electron.js", // important
                "stopOnEntry": false,
                "args": [],
                "cwd": "${workspaceRoot}/App/", 
                "runtimeExecutable": "electron 路径",
                "runtimeArgs": [],
                "env": { }, 
                "sourceMaps": true
            }
        ]
    }
 Mac 配置如下：
     {
        "version": "0.2.0",
        "configurations": [

            {
                "name": "Launch Electron App",
                "type": "node",
                "program": "${workspaceRoot}/App/electron.js", // important
                "stopOnEntry": false,
                "args": [],
                "cwd": "${workspaceRoot}/App/", 
                "runtimeExecutable": "electron路径/Electron/Contents/MacOS/Electron",
                "runtimeArgs": [],
                "env": { }, 
                "sourceMaps": true
            }
        ]
    }
    
前端呼叫後端流程
ex. 由container.component 透過 protocol.service 再傳至 protocolfunction 到system的function
但此方式有個問題, system function無法callback參數值只能回傳是否有error, 修正方式callback obj裡面包含err和param

後端通知前端流程
Component 透過ElectronEventService來收到後端的通知 -->