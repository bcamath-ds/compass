/***********************************************************************
*  This code is part of Compass.
*
*  Copyright (C) 2000-2013 Andrew Makhorin, Department for Applied
*  Informatics, Moscow Aviation Institute, Moscow, Russia. All rights
*  reserved. E-mail: <gkobeaga@bcamath.org>.
*
*  Compass is free software: you can redistribute it and/or modify it
*  under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Compass is distributed in the hope that it will be useful, but WITHOUT
*  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
*  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
*  License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Compass. If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/

struct op_eacp
{ /* OP Genetic Algorithm control parameters */
  int msg_lev;              /* message level: */
#define COMPASS_MSG_OFF       0  /* no output */
#define COMPASS_MSG_ERR       1  /* warning and error messages only */
#define COMPASS_MSG_ON        2  /* normal output */
#define COMPASS_MSG_ALL       3  /* full output */
#define COMPASS_MSG_DBG       4  /* debug output */
  double tm_start;              /* starting timestamp */
  double tm_end;                /* ending timestamp */
  double tm_lim;               /* time limit (milliseconds) */
  int it_lim;               /* time limit (milliseconds) */
  int it;
  int pop_size;
  int len_improve1;
  int len_improve2;
  double pmut;
  int nparsel;
  double pinit;
  int d2d;
};
