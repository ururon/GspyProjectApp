import { Routes } from '@angular/router';
import { NavRoutes } from './layout/nav/nav.routes';
import { ContainerRoutes } from './layout/container/container.routes';

export const routes: Routes = [ 
    ...ContainerRoutes, 
    ...NavRoutes
];