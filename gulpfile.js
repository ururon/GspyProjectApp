// 获取依赖
var gulp = require('gulp'),
    childProcess = require('child_process'),
    electron = require('electron-prebuilt');
    
// 创建 gulp 任务
// gulp.task('run', function () {
//     childProcess.spawn(electron, ['.'], {stdio:'inherit'});
// });

let ts = require('gulp-typescript');
let uglify = require('gulp-uglify');
let tsProject = ts.createProject('tsconfig.json', {typescript: require('typescript')});
var install = require("gulp-install");
let sass = require('gulp-sass');
let rename = require('gulp-rename');

gulp.task('default',['html']);


gulp.task('html',['css'],function(){
    return gulp.src('./src/angular/**/*.html').pipe(gulp.dest('./App'));
});

gulp.task('css',['scss'],function(){
    return gulp.src(['./src/angular/**/*.css']).pipe(gulp.dest('./App'));
});

gulp.task('scss',['sass'],function(){
    return gulp.src('./src/angular/assets/scss/**/*.scss').pipe(rename({ dirname: '' })).pipe(sass().on('error', sass.logError)).pipe(gulp.dest('./App/css'));
});

gulp.task('sass',['ts'],function(){
    return gulp.src(['./src/angular/**/*.scss','!./src/angular/assets/scss/**/*.scss']).pipe(sass().on('error', sass.logError)).pipe(gulp.dest('./App'));
});

gulp.task('ts',['js'],function(){
    let tsResult = tsProject.src().pipe(tsProject());    
    return tsResult.js.pipe(uglify()).pipe(gulp.dest('./App/'));
});

gulp.task('js',['backend'],function(){
    return gulp.src(['./src/angular/assets/js/**/*']).pipe(gulp.dest('./App/js'));
});

gulp.task('backend',['reSystem'],function(){
    return gulp.src(['./src/backend/**/*','!./src/backend/data/**/*']).pipe(gulp.dest('App/backend'));
});

gulp.task('reSystem',['reTs'],function(){
   let src = ['index.html','package.json','electron.js','windows.js','systemjs.config.js']
   return gulp.src(src).pipe(gulp.dest('./App'));
});

gulp.task('reTs',['Nodemodules'],function(){
    let tsResult = tsProject.src().pipe(tsProject());    
    return tsResult.js.pipe(uglify()).pipe(gulp.dest('./App/'));
});

gulp.task('Nodemodules',['install'],function(){ 
    let src = [
        'node_modules/core-js/**/*',
        'node_modules/reflect-metadata/**/*',
        'node_modules/systemjs/**/*',
        'node_modules/zone.js/**/*',    
        'node_modules/@angular/**/*',  
        'node_modules/rxjs/**/*', 
        'node_modules/lodash/**/*', 
        'node_modules/moment/**/*',  
        'node_modules/@ngrx/**/*',  
        'node_modules/angulartics2/**/*', 
        'node_modules/ng2-translate/**/*', 
        'node_modules/traceur/**/*',  
        'node_modules/nedb/**/*',
        'node_modules/@angular2-material/**/*',
        'node_modules/hammerjs/**/*',
        'node_modules/mkdirp/**/*',
        'node_modules/underscore/**/*',
        'node_modules/binary-search-tree/**/*',
        'node_modules/async/**/*',
        'node_modules/growly/**/*',
        'node_modules/isexe/**/*',
        'node_modules/semver/**/*',
        'node_modules/jquery/**/*',
        'node_modules/jqueryui/**/*',
        'node_modules/which/**/*',
        'node_modules/shellwords/**/*', 
        'node_modules/ng2-pagination/**/*',
        'node_modules/angular-2-local-storage/**/*', 
        'node_modules/adm-zip/**/*',
        'node_modules/getmac/**/*',
        'node_modules/request/**/*',
        'node_modules/extend/**/*',
        'node_modules/tough-cookie/**/*', 
        'node_modules/json-stringify-safe/**/*', 
        'node_modules/safe-buffer/**/*', 
        'node_modules/hawk/**/*',
        'node_modules/boom/**/*',
        'node_modules/hoek/**/*',
        'node_modules/sntp/**/*',
        'node_modules/cryptiles/**/*',
        'node_modules/aws-sign2/**/*',
        'node_modules/aws4/**/*',
        'node_modules/http-signature/**/*',
        'node_modules/assert-plus/**/*',
        'node_modules/sshpk/**/*',
        'node_modules/asn1/**/*',
        'node_modules/jsprim/**/*',
        'node_modules/verror/**/*',
        'node_modules/json-schema/**/*',
        'node_modules/mime-types/**/*',
        'node_modules/mime-db/**/*',
        'node_modules/stringstream/**/*',
        'node_modules/caseless/**/*',
        'node_modules/forever-agent/**/*',
        'node_modules/form-data/**/*',
        'node_modules/combined-stream/**/*',
        'node_modules/delayed-stream/**/*',
        'node_modules/asynckit/**/*',
        'node_modules/isstream/**/*',
        'node_modules/is-typedarray/**/*',
        'node_modules/qs/**/*',
        'node_modules/har-validator/**/*',
        'node_modules/har-schema/**/*',
        'node_modules/ajv/**/*',
        'node_modules/json-stable-stringify/**/*',
        'node_modules/co/**/*',
        'node_modules/uuid/**/*',
        'node_modules/tunnel-agent/**/*',
        'node_modules/performance-now/**/*',
        'node_modules/oauth-sign/**/*',
        'node_modules/extsprintf/**/*',
        'node_modules/extract-opts/**/*', 
        'node_modules/editions/**/*', 
        'node_modules/typechecker/**/*',
        'node_modules/eachr/**/*', 
        'node_modules/universal-analytics/**/*',       
        'node_modules/electron-notifications/**/*',
        'node_modules/electron-is/**/*',
        'node_modules/electron-is-dev/**/*',
   ]
    return gulp.src(src, { base: 'node_modules' })
        .pipe(gulp.dest('App/node_modules'));

});

gulp.task('install',() => {
   return gulp.src(['./package.json'])
    .pipe(install({noOptional: true}));
}); 