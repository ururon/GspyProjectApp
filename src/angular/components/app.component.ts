import { Component } from '@angular/core';

@Component({
    selector: 'sm-app',
    templateUrl : './components/app.component.html',
    //template: '<h1>我的第一个 Angular 应用</h1>',
    styleUrls: ['./components/app.component.css']
})

export class AppComponent{
    title = "app";
}