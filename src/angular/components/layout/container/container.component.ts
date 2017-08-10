declare var System;
import { Component ,OnInit} from '@angular/core';
import { ElectronEventService } from '../../../services/libs/electron/index';
import { protocolService } from '../../../services/service/protocol.service';
let evtVar = System._nodeRequire('./backend/generic/eventVariates');
let funcVar = System._nodeRequire('./backend/generic/functionVariates');

@Component({
    selector: 'sm-container',
    templateUrl : './components/layout/container/container.component.html',
    styleUrls: ['./components/layout/container/container.component.css'],
    providers: [protocolService]
})

export class ContainerComponent implements OnInit{

    VID:string="";
    PID:string="";
    Status:string="";
    DeviceName:string="";

    constructor(private protocol: protocolService){
        console.log('container loading complete');
    }

    ngOnInit(){
        //收到後端通知
        ElectronEventService.on('icpEvent').subscribe((e: any) => {
            var obj = JSON.parse(e.detail);
            if (obj.Func === evtVar.EventTypes.RefreshDevice) {
                this.VID = obj.Param.VID;
                this.PID = obj.Param.PID;
                this.DeviceName = obj.Param.DeviceName;
                if(obj.Param.PlugType==1)
                    this.Status="USB裝置插入"
                else if(obj.Param.PlugType==0)
                    this.Status="USB裝置移除"
                let obj1 = {
                    Type : funcVar.Types.System,
                    SN:"111111",
                    Func:funcVar.Names.abctest,
                    Param:"111"
                }
                console.log(JSON.stringify(obj1));
                //呼叫後端函數
                this.protocol.RunSetFunction(obj1).then((data) => {
                        console.log("Container RunSetFunction:" + data);
                });
            }
        });
    }
}