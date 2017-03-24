/***********************************************************************
*  This code is part of Compass.
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

#include "compass.h"
#include "op.h"
#include "env.h"


void compass_hash_init(compass_prob *prob)
{ SHA256_Init(prob->ctx);

  SHA256_Final(prob->hash, prob->ctx);

#if 0
  compass_hash_print(prob->hash);
#endif

  return;
}

void compass_hash_update(compass_prob *prob, int flag)
{

  if (flag == HASH_UPDATE_NAME)
  { SHA256_Update(prob->ctx, prob->name, strlen(prob->name));
  }
  else if (flag == HASH_UPDATE_OPSCORE)
  { int i;
    char str[15];
    for (i=0; i<prob->n; i++)
    { sprintf(str, "%f", prob->op->s[i]);
      SHA256_Update(prob->ctx, str, strlen(str));
    }
  }
  else if (flag == HASH_UPDATE_OPD0)
  { char str[25];
    sprintf(str, "%f", prob->op->d0);

    SHA256_Update(prob->ctx, str, strlen(str));
    SHA256_Final(prob->hash, prob->ctx);
  }
  SHA256_Final(prob->hash, prob->ctx);

#if 0
  compass_hash_print(prob->hash);
#endif

  return;
}

void compass_hash_update_time(compass_prob *prob, double tm)
{ char str[25];
  sprintf(str, "%f", tm);

  SHA256_Update(prob->ctx, str, strlen(str));
  SHA256_Final(prob->hash, prob->ctx);

#if 0
  compass_hash_print(prob->hash);
#endif

  return;
}

void compass_hash_print(unsigned char hash[])
{ int i;
  xprintf("  Problem hash: ");
  for (i=0; i < 32; i++)
    xprintf("%02x",hash[i]);
  xprintf("\n");
}
