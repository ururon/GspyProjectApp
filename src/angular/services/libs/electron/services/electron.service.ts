/*!---------------------------------------------------------
* Copyright (C) Genius Corporation. All rights reserved.
*--------------------------------------------------------*/

import {Observable} from 'rxjs/Observable';
import 'rxjs/add/observable/fromEvent';

declare var window: any;

export class ElectronEventService {

  public static on(name: string): Observable<any> {
    return Observable.fromEvent(window, name);
  }
  
}
