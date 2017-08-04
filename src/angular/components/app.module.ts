import { NgModule } from '@angular/core';
import { BrowserModule } from '@angular/platform-browser';
import { AppComponent } from './app.component';
import { NavComponent } from './layout/nav/nav.component';
import { ContainerComponent } from './layout/container/container.component';
import { APP_BASE_HREF } from '@angular/common';
import { RouterModule } from '@angular/router';
// app routes
import { routes } from './app.routes'; 
//let routerModule = RouterModule.forRoot(routes);

//routerModule = RouterModule.forRoot(routes, {useHash: true});
@NgModule({
    declarations: [ AppComponent,
                    NavComponent,
                    ContainerComponent
                  ],
    imports: [
                BrowserModule
                //routerModule
            ],
    bootstrap: [AppComponent],
    providers: [{
                provide: APP_BASE_HREF,
                useValue: '<%= APP_BASE %>'
                }]
                    
})

export class AppModule { }