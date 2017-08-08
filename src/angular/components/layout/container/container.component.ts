declare var System;
import { Component ,OnInit} from '@angular/core';
import { ElectronEventService } from '../../../services/libs/electron/index';
let evtVar = System._nodeRequire('./backend/generic/eventVariates');

@Component({
    selector: 'sm-container',
    templateUrl : './components/layout/container/container.component.html',
    styleUrls: ['./components/layout/container/container.component.css']
})

export class ContainerComponent implements OnInit{

    VID:string="";
    PID:string="";

    ngOnInit(){
        ElectronEventService.on('icpEvent').subscribe((e: any) => {
            var obj = JSON.parse(e.detail);
            if (obj.Func === evtVar.EventTypes.RefreshDevice) {
                this.VID = obj.Param.VID;
                this.PID = obj.Param.PID;
                console.log('Robert3');
            }
            console.log('Robert'+JSON.stringify(obj));
        });
    }
}