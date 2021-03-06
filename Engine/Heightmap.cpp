#include "heightmap.h"

#include <assert.h>
#include <errno.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "GameMath.h"

void Heightmap_print(Heightmap *map)
{
    printf("Heightmap {\n");
    printf(" %ld x %ld\n", map->width, map->height);
    printf(" m_map: %p\n", map->map);
    printf(" m_map[0]: %f\n", map->map[0]);
    printf(" m_map[last]: %f\n", map->map[map->width*map->height - 1]);
    printf(" min: %f\n", map->minZ);
    printf(" max: %f\n", map->maxZ);

    size_t histogram[11] = {0};
    size_t i;

    for (i=0; i<map->width*map->height; ++i) {
        float val = map->map[i];
        val *= 10;
        if (val >10) {
            printf("WAD: %f\n", map->map[i]);
            histogram[10]++;
        } else {
            histogram[(int)val]++;
        }
    }

    printf(" histogram {\n");
    printf(" 0.0 - 0.1 : %lu,\n", histogram[0]);
    printf(" 0.1 - 0.2 : %lu,\n", histogram[1]);
    printf(" 0.2 - 0.3 : %lu,\n", histogram[2]);
    printf(" 0.3 - 0.4 : %lu,\n", histogram[3]);
    printf(" 0.4 - 0.5 : %lu,\n", histogram[4]);
    printf(" 0.5 - 0.6 : %lu,\n", histogram[5]);
    printf(" 0.6 - 0.7 : %lu,\n", histogram[6]);
    printf(" 0.7 - 0.8 : %lu,\n", histogram[7]);
    printf(" 0.8 - 0.9 : %lu,\n", histogram[8]);
    printf(" 0.9 - 1.0 : %lu,\n", histogram[9]);
    printf(" else : %lu,\n", histogram[10]);
    printf(" }\n");

    printf("}\n");
}

Heightmap *Heightmap_read(const char *filename)
{
    Heightmap *map = NULL;
    FILE *fd = fopen(filename, "r");
    if (!fd) {
        printf("Unable to open file %s : %s\n", filename, strerror(errno));
        return map;
    }

    map = new Heightmap();
    map->normal_map = NULL;

    fscanf(fd, "%lu ", &map->width);
    fscanf(fd, "%lu", &map->height);

    map->map = new float[map->width*map->height];
    memset(map->map, 0, map->width*map->height*sizeof(float));

    float *ptr = map->map;

    size_t i;

    for (i=0; i<map->height*map->width; ++i) {
        fscanf(fd, "%f", ptr);
        map->minZ = MIN(map->minZ, *ptr);
        map->maxZ = MAX(map->maxZ, *ptr);
        ++ptr;
    }

    fclose(fd);

    return map;
}

void maps_delete(Heightmap *map)
{
    if (map->map) {
        delete[] map->map;
    }
    if (map->normal_map) {
        delete[] map->normal_map;
    }
    delete map;
}

void Heightmap_normalize(Heightmap *map)
{
    int i;
    for (i=0; i<map->height*map->width; ++i) {
        map->map[i] /= map->maxZ;
    }
    map->maxZ /= map->maxZ;
    map->minZ /= map->maxZ;
}

void Heightmap_calculate_normals(Heightmap *map)
{
    int x, y;

    map->normal_map = new float[3*map->width*map->height];
    memset(map->normal_map, 0, 3*map->width*map->height*sizeof(float));

      for (y=0; y<map->height; ++y) {
          for (x=0; x<map->width; ++x) {
              int k = 3*(map->height*y + x);
  
              // corner cases
              if (x == 0 || x == map->width-1 || y == 0 || y == map->height-1) {
                  map->normal_map[k+0] = 0;
                  map->normal_map[k+1] = 0;
                  map->normal_map[k+2] = 1.0;
                  continue;
              }
  
              // dx: Sobel filter
              // -1  0  1
              // -2  0  2
              // -1  0  1
              //
              // dy: Sobel filter
              // -1 -2 -1
              //  0  0  0
              //  1  2  1
              float tl = Heightmap_get(map, x-1, y-1);
              float l = Heightmap_get(map, x-1, y );
              float bl = Heightmap_get(map, x-1, y+1);
              float b = Heightmap_get(map, x, y+1);
              float br = Heightmap_get(map, x+1, y+1);
              float r = Heightmap_get(map, x+1, y );
              float tr = Heightmap_get(map, x+1, y-1);
              float t = Heightmap_get(map, x, y-1);
  
              float dx = tr + 2 * r + br - tl - 2 * l - bl;
              float dy = bl + 2 * b + br - tl - 2 * t - tr;
  
              // trial & error value.
              float str = 32.0;
  
              float length = sqrtf(dx*dx + dy*dy + 1.0/str*1.0/str);
  
              map->normal_map[k+0] = dx / length;
              map->normal_map[k+1] = dy / length;
              map->normal_map[k+2] = 1.0 / (str*length);
          }
      }

    //int	offs = 0;                   // offset to normalMap
    //float scale = 10.0;

    //for ( int i = 0; i < map->height; i++ )
    //    for ( int j = 0; j < map->width; j++ )
    //    {
    //        int k = 3*(map->height*i + j);
    //        if (i == 0 || i == map->width-1 || j == 0 || j == map->height-1) {
    //                              map->normal_map[k+0] = 0;
    //                              map->normal_map[k+1] = 0;
    //                              map->normal_map[k+2] = 1.0;
    //                              continue;
    //                          }

    //        // convert height values to [0,1] range
    //        float	c  = Heightmap_get(map, i,   j);
    //        float	cx = Heightmap_get(map, i,   j+1);
    //        float	cy = Heightmap_get(map, i+1, j);

    //        // find derivatives
    //        float	dx = (c - cx) * scale;
    //        float	dy = (c - cy) * scale;

    //        // normalize
    //        float	len = (float) sqrt ( dx*dx + dy*dy + 1 );

    //        // get normal
    //        float	nx = dy   / len;
    //        float	ny = -dx  / len;
    //        float	nz = 1.0f / len;

    //        // now convert to color and store in map
    //        map->normal_map[k+0] = nx;
    //        map->normal_map[k+1] = ny;
    //        map->normal_map[k+2] = nz;
    //    }
}

void Heightmap_get_normal(Heightmap *map, int x, int y, float *nx, float *ny, float *nz)
{
    assert(x >= 0 && x < map->width);
    assert(y >= 0 && x < map->height);
    int k = 3*(map->height*y + x);
    *nx = map->normal_map[k+0];
    *ny = map->normal_map[k+1];
    *nz = map->normal_map[k+2];
}

float Heightmap_get(Heightmap *map, int x, int y)
{
    assert((map->height*y + x) < (map->height*map->width));
    assert((map->height*y + x) >= 0);
    return (map->map[map->height*y + x]);
}