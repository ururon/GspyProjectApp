/**
 * System configuration for Angular samples
 * Adjust as necessary for your application needs.
 */
(function (global) {
  System.config({
    paths: {
      // paths serve as alias
      'npm:': 'node_modules/',
      'ng2-translate':  'node_modules/ng2-translate/bundles/ng2-translate.umd.js',
      "@angular2-material/*": "node_modules/@angular2-material/*", 
    },
    // map tells the System loader where to look for things
    map: {
      // our app is within the app folder
      App: '',
      // angular bundles
      '@angular/core': 'npm:@angular/core/bundles/core.umd.js',
      '@angular/common': 'npm:@angular/common/bundles/common.umd.js',
      '@angular/compiler': 'npm:@angular/compiler/bundles/compiler.umd.js',
      '@angular/platform-browser': 'npm:@angular/platform-browser/bundles/platform-browser.umd.js',
      '@angular/platform-browser-dynamic': 'npm:@angular/platform-browser-dynamic/bundles/platform-browser-dynamic.umd.js',
      '@angular/http': 'npm:@angular/http/bundles/http.umd.js',
      '@angular/router': 'npm:@angular/router/bundles/router.umd.js',
      '@angular/forms': 'npm:@angular/forms/bundles/forms.umd.js',  
      '@angular/material': 'npm:@angular/material/bundles/material.umd.js',
      "hammerjs": "npm:hammerjs", 
      'ng2-translate': 'npm:ng2-translate/bundles/ng2-translate.umd.js', 
      'angulartics2':'npm:angulartics2',
      'jquery':'npm:jquery',
      'jqueryui':'npm:jqueryui',
      'angular-2-local-storage':'npm:angular-2-local-storage',
      'ng2-pagination':'npm:ng2-pagination',
      '@ngrx': 'npm:@ngrx', 
      'traceur':'npm:traceur/src/traceur.js', 
      'rxjs': 'npm:rxjs',
      'angular-in-memory-web-api': 'npm:angular-in-memory-web-api',
      'lodash': 'npm:lodash',
      'moment': 'npm:moment'
    },
    // packages tells the System loader how to load when no filename and/or no extension
    packages: {
      App: {
        main: './components/main.js',
        defaultExtension: 'js'
      },
      rxjs: {
        defaultExtension: 'js'
      },
      'angular-in-memory-web-api': {
        main: './index.js',
        defaultExtension: 'js'
      },
      'moment': {
        main: './moment.js',
        defaultExtension: 'js'
      },
      'lodash': {
        main: './index.js',
        defaultExtension: 'js'
      },
      '@ngrx/store':  {
         main: 'bundles/store.umd.js',  
         format: 'cjs'           
      },
      '@ngrx/core': {  
          main: 'bundles/core.umd.js',    
          format: 'cjs'            
      }, 
      'angulartics2':  {  
          main: 'dist/index.js',          
          defaultExtension: 'js'   
      },
      'hammerjs': {
         main: 'hammer.js',  
         defaultExtension: 'js'  
      },
      'ngx-modal': { 
        'main': 'index.js',
        'defaultExtension': 'js' 
      },
      'jquery':{
        main:'dist/jquery.js',        
        defaultExtendsion:'js'
      },
      'jqueryui':{
        main:'jquery-ui.js',
        defaultExtendsion:'js'
      },
      'ng2-pagination':{
        main: 'dist/ng2-pagination.js',
        defaultExtension: 'js'
      },
      'angular-2-local-storage':{
        main: 'dist/index.js',
        defaultExtension: 'js'  
      } 
    }
  });
})(this);