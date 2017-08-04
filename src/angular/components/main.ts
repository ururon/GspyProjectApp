// import 'reflect-metadata';
// import 'zone.js';

// import { platformBrowserDynamic } from '@angular/platform-browser-dynamic';
// import { AppModule } from './app.module';

// platformBrowserDynamic().bootstrapModule(AppModule).catch(err => console.error(err));

import {enableProdMode} from '@angular/core';  
import { platformBrowserDynamic }    from '@angular/platform-browser-dynamic';  
import { AppModule } from './app.module'; 
 
enableProdMode();

platformBrowserDynamic().bootstrapModule(AppModule)
.catch((err:any) => console.error(err));
 