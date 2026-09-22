/*
 * Parameters.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


/******************************************************************************
 * The global parameters
 */

/*
// Small
double cell_Radius = 200; // 5 m

double cell_MinHeight = 2000; // 30 m
double cell_MaxHeight = 20000; // 200 m\
int cell_NBinsHeight = (cell_MaxHeight - cell_MinHeight)/(2*cell_Radius) + 1; 
    
double cell_MinDistance = 0; // 0 m
double cell_MaxDistance = 20000; // 200 m
int cell_NBinsDistance = (cell_MaxDistance - cell_MinDistance)/(2*cell_Radius) + 1;
*/


// Large
bool cell_LineOnly = false;

double cell_Radius = 500; // 5 m
 
double cell_MinHeight = 2000; // 20 m
double cell_MaxHeight = 20000; // 200 m
int cell_NBinsHeight = (cell_MaxHeight - cell_MinHeight)/(2*cell_Radius) + 1; 
   
double cell_MinDistance = 0; // 0 m
double cell_MaxDistance = 20000; // 200 m
int cell_NBinsDistance = (cell_MaxDistance - cell_MinDistance)/(2*cell_Radius) + 1;


bool auxcell_Use = true;
double auxcell_Radius = 75; // 0.75 m 

double auxcell_MinHeight = 750; // 7.5 m
double auxcell_MaxHeight = 750;
int auxcell_NBinsHeight = (auxcell_MaxHeight - auxcell_MinHeight)/(2*auxcell_Radius) + 1;

double auxcell_MinDistance = 0; // 0 m
double auxcell_MaxDistance = 3000; // 300 m 
int auxcell_NBinsDistance = (auxcell_MaxDistance - auxcell_MinDistance)/(2*auxcell_Radius) + 1; // at least 2!

