import { Component ,OnInit} from '@angular/core';
import {protocolService} from '../services/service/protocol.service'

@Component({
    selector: 'sm-app',
    templateUrl : './components/app.component.html',
    //template: '<h1>我的第一个 Angular 应用</h1>',
    styleUrls: ['./components/app.component.css'],
    providers: [protocolService]
})

export class AppComponent implements OnInit{

    constructor(private protocol: protocolService){
        console.log('app loading complete')
    }

    title = "app";
    
    ngOnInit() {
    }
}