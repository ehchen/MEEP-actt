/* Copyright (C) 2005-2014 Massachusetts Institute of Technology
%
%  This program is free software; you can redistribute it and/or modify
%  it under the terms of the GNU General Public License as published by
%  the Free Software Foundation; either version 2, or (at your option)
%  any later version.
%
%  This program is distributed in the hope that it will be useful,
%  but WITHOUT ANY WARRANTY; without even the implied warranty of
%  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%  GNU General Public License for more details.
%
%  You should have received a copy of the GNU General Public License
%  along with this program; if not, write to the Free Software Foundation,
%  Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex>
#include <string.h>

#include "meep_internals.hpp"

namespace meep {

ivec grid_volume::round_vec(const vec &p) const {
  ivec result(dim);
  LOOP_OVER_DIRECTIONS(dim, d)
    result.set_direction(d, my_round(p.in_direction(d) * 2 * a));
  return result;
}

void grid_volume::set_origin(const ivec &o) {
  io = o;
  origin = operator[](io); // adjust origin to match io
}

void grid_volume::set_origin(direction d, int o) {
  io.set_direction(d, o);
  origin = operator[](io); // adjust origin to match io
}

void grid_volume::set_origin(const vec &o) {
  set_origin(round_vec(o));
}

const char *dimension_name(ndim dim) {
  switch (dim) {
  case D1: return "1D";
  case D2: return "2D";
  case D3: return "3D";
  case Dcyl: return "Cylindrical";
  }
  return "Error in dimension_name";
}

const char *direction_name(direction d) {
  switch (d) {
  case X: return "x";
  case Y: return "y";
  case Z: return "z";
  case R: return "r";
  case P: return "phi";
  case NO_DIRECTION: return "no_direction";
  }
  return "Error in direction_name";
}

const char *component_name(component c) {
  if (is_derived(int(c))) return component_name(derived_component(c));
  switch (c) {
  case Er: return "er";
  case Ep: return "ep";
  case Ez: return "ez";
  case Hr: return "hr";
  case Hp: return "hp";
  case Hz: return "hz";
  case Ex: return "ex";
  case Ey: return "ey";
  case Hx: return "hx";
  case Hy: return "hy";
  case Dx: return "dx";
  case Dy: return "dy";
  case Dz: return "dz";
  case Dr: return "dr";
  case Dp: return "dp";
  case Bx: return "bx";
  case By: return "by";
  case Bz: return "bz";
  case Br: return "br";
  case Bp: return "bp";
  case Dielectric: return "eps";
  case Permeability: return "mu";
  }
  return "Error in component_name";
}

const char *component_name(derived_component c) {
  if (!is_derived(int(c))) return component_name(component(c));
  switch (c) {
  case Sr: return "sr";
  case Sp: return "sp";
  case Sz: return "sz";
  case Sx: return "sx";
  case Sy: return "sy";
  case EnergyDensity: return "energy";
  case D_EnergyDensity: return "denergy";
  case H_EnergyDensity: return "henergy";
  }
  return "Error in component_name";
}

const char *component_name(int c) {
  return (is_derived(c) ? component_name(derived_component(c))
	  : component_name(component(c)));
}

component first_field_component(field_type ft) {
  switch (ft) {
  case E_stuff: return Ex;
  case H_stuff: return Hx;
  case D_stuff: return Dx;
  case B_stuff: return Bx;
  default: abort("bug - only E/H/D/B stuff have components"); 
  }
}

vec min(const vec &vec1, const vec &vec2) {
  vec m(vec1.dim);
  LOOP_OVER_DIRECTIONS(vec1.dim, d)
    m.set_direction(d, min(vec1.in_direction(d), vec2.in_direction(d)));
  return m;
}

vec max(const vec &vec1, const vec &vec2) {
  vec m(vec1.dim);
  LOOP_OVER_DIRECTIONS(vec1.dim, d)
    m.set_direction(d, max(vec1.in_direction(d), vec2.in_direction(d)));
  return m;
}

ivec min(const ivec &ivec1, const ivec &ivec2) {
  ivec m(ivec1.dim);
  LOOP_OVER_DIRECTIONS(ivec1.dim, d)
    m.set_direction(d, min(ivec1.in_direction(d), ivec2.in_direction(d)));
  return m;
}

ivec max(const ivec &ivec1, const ivec &ivec2) {
  ivec m(ivec1.dim);
  LOOP_OVER_DIRECTIONS(ivec1.dim, d)
    m.set_direction(d, max(ivec1.in_direction(d), ivec2.in_direction(d)));
  return m;
}

volume::volume(const vec &vec1, const vec &vec2) {
  min_corner = min(vec1, vec2);
  max_corner = max(vec1, vec2);
  dim = vec1.dim; 
}

volume::volume(const vec &pt) {
  dim = pt.dim; 
  min_corner = pt;
  max_corner = pt;
}

double volume::computational_volume() const {
  double vol = 1.0; 
  LOOP_OVER_DIRECTIONS(dim,d) vol *= in_direction(d);
  return vol;
}

double volume::integral_volume() const {
  double vol = 1.0; 
  LOOP_OVER_DIRECTIONS(dim, d)
    if (in_direction(d) != 0.0) vol *= in_direction(d);
  if (dim == Dcyl) vol *= pi * (in_direction_max(R) + in_direction_min(R));
  return vol;
}

double volume::full_volume() const {
  double vol = computational_volume(); 
  if (dim == Dcyl) vol *= pi * (in_direction_max(R) + in_direction_min(R));
  return vol;
}

double volume::diameter() const {
  double diam = 0.0;
  LOOP_OVER_DIRECTIONS(dim,d) {
    diam = max(diam, in_direction(d));
  }
  return diam;
}

volume volume::intersect_with(const volume &a) const {
  if (a.dim != dim) abort("Can't intersect volumes of dissimilar dimensions.\n");
  volume result(dim);
  LOOP_OVER_DIRECTIONS(dim, d) {
    double minval = max(in_direction_min(d), a.in_direction_min(d));
    double maxval = min(in_direction_max(d), a.in_direction_max(d));
    if (minval > maxval)
      return volume(zero_vec(dim), zero_vec(dim));
    result.set_direction_min(d, minval);
    result.set_direction_max(d, maxval);
  }
  return result;
}

bool volume::intersects(const volume &a) const {
  if (a.dim != dim) abort("Can't intersect volumes of dissimilar dimensions.\n");
  LOOP_OVER_DIRECTIONS(dim, d) {
    double minval = max(in_direction_min(d), a.in_direction_min(d));
    double maxval = min(in_direction_max(d), a.in_direction_max(d));
    if (minval > maxval)
      return false;
  }
  return true;
}

// Return normal direction to grid_volume, if the grid_volume is dim-1 dimensional;
// otherwise, return NO_DIRECTION.
direction volume::normal_direction() const {
  direction d = NO_DIRECTION;
  switch (dim) {
  case D1: d = Z; break;
  case D2:
    if (in_direction(X) == 0 && in_direction(Y) > 0)
      d = X;
    else if (in_direction(X) > 0 && in_direction(Y) == 0)
      d = Y;
    break;
  case Dcyl:
    if (in_direction(R) == 0 && in_direction(Z) > 0)
      d = R;
    else if (in_direction(R) > 0 && in_direction(Z) == 0)
      d = Z;
    break;
  case D3: {
    bool zx = in_direction(X) == 0;
    bool zy = in_direction(Y) == 0;
    bool zz = in_direction(Z) == 0;
    if (zx && !zy && !zz) d = X;
    else if (!zx && zy && !zz) d = Y;
    else if (!zx && !zy && zz) d = Z;
    break;
  }
  }
  return d;
}

/* Used for n=0,1,2 nested loops in macros.  We should arrange
   the ordering so that this gives most efficient traversal of
   a field array, where n=2 is the innermost loop. */
static direction yucky_dir(ndim dim, int n) {
  if (dim == Dcyl)
    switch (n) {
    case 0: return P;
    case 1: return R;
    case 2: return Z;
    }
  else if (dim == D2)
    return (direction) ((n + 2) % 3); /* n = 0,1,2 gives Z, X, Y */
  return (direction) n ;
}

int ivec::yucky_val(int n) const {
  if (has_direction(dim, yucky_dir(dim, n)))
    return in_direction(yucky_dir(dim, n));
  return 0;
}

int grid_volume::yucky_num(int n) const {
  if (has_direction(dim, yucky_dir(dim, n)))
    return num_direction(yucky_dir(dim, n));
  return 1;
}

direction grid_volume::yucky_direction(int n) const {
  return yucky_dir(dim, n);
}

volume grid_volume::surroundings() const {
  return volume(operator[](little_corner()), 
			  operator[](big_corner()));
}

volume grid_volume::interior() const {
  return volume(operator[](little_corner()), 
			  operator[](big_corner() - one_ivec(dim) * 2));
}

void grid_volume::update_ntot() {
  the_ntot = 1;
  LOOP_OVER_DIRECTIONS(dim, d) the_ntot *= num[d%3] + 1;
}

void grid_volume::set_num_direction(direction d, int value) {
  num[d%3] = value; num_changed();
}

grid_volume::grid_volume(ndim td, double ta, int na, int nb, int nc) {
  dim = td; a = ta; inva = 1.0 / ta;
  num[0] = na;
  num[1] = nb;
  num[2] = nc;
  num_changed();
  set_origin(zero_vec(dim));
}

component grid_volume::eps_component() const {
  switch (dim) {
  case D1: return Hy;
  case D2: return Hz;
  case D3: return Dielectric;
  case Dcyl: return Hp;
  }
  abort("Unsupported dimensionality eps.\n");
  return Ex;
}

vec grid_volume::yee_shift(component c) const {
  return operator[](iyee_shift(c));
}

/* Return array offsets to average with a given array location of c in
   order to get c on the "centered" grid.  Then, to get the
   centered grid point i, you should average c over the four
   locations: i, i+offset1, i+offset2, i+offset1+offset2. 
   (offset2, and possibly offset1, may be zero if only 2 or 1
   locations need to be averaged). */
void grid_volume::yee2cent_offsets(component c, int &offset1, int &offset2) const {
  offset1 = offset2 = 0;
  LOOP_OVER_DIRECTIONS(dim,d) {
    if (!iyee_shift(c).in_direction(d)) {
      if (offset2) 
	abort("weird yee shift for component %s", component_name(c));
      if (offset1) offset2 = stride(d);
      else offset1 = stride(d);
    }
  }
}

/* Same as yee2cent_offsets, but averages centered grid to get c */
void grid_volume::cent2yee_offsets(component c, int &offset1, int &offset2) const {
  yee2cent_offsets(c, offset1, offset2);
  offset1 = -offset1;
  offset2 = -offset2;
}

bool volume::contains(const vec &p) const {
  LOOP_OVER_DIRECTIONS(dim,d) {
    if (p.in_direction(d) > in_direction_max(d) ||
        p.in_direction(d) < in_direction_min(d)) return false;
  }
  return true;
}

bool volume::contains(const volume &a) const {
  return contains(a.get_min_corner()) && contains(a.get_max_corner());
}

bool grid_volume::contains(const ivec &p) const {
  // containts returns true if the grid_volume has information about this grid
  // point.
  const ivec o = p - io;
  LOOP_OVER_DIRECTIONS(dim, d)
    if (o.in_direction(d) < 0 || o.in_direction(d) >= (num_direction(d)+1)*2)
      return false;
  return true;
}

bool grid_volume::contains(const vec &p) const {
  // containts returns true if the grid_volume has any information in it
  // relevant to the point p.  Basically has is like owns (see below)
  // except it is more lenient, in that more than one lattice may contain a
  // given point.
  const vec o = p - origin;
  LOOP_OVER_DIRECTIONS(dim, d)
    if (o.in_direction(d) < -inva || o.in_direction(d) > num_direction(d)*inva+inva)
      return false;
  return true;
}

/* Compute the corners (cs,ce) of the ib-th boundary for component c,
   returning true if ib is a valid index (ib = 0..#boundaries-1).  The
   boundaries are all the points that are in but not owned by the
   grid_volume, and are a set of *disjoint* regions.  The main purpose of
   this function is currently to support the LOOP_OVER_NOT_OWNED
   macro.  (In the future, it may be used for other
   boundary-element-type computations, too.) */
bool grid_volume::get_boundary_icorners(component c, int ib,
				   ivec *cs, ivec *ce) const {
  ivec cl(little_corner() + iyee_shift(c));
  ivec cb(big_corner() + iyee_shift(c));
  ivec clo(little_owned_corner(c));
  ivec cbo(big_corner() - iyee_shift(c));
  *cs = cl;
  *ce = cb;
  bool ib_found = false;
  int jb = 0;
  LOOP_OVER_DIRECTIONS(dim, d) {
    if (cl.in_direction(d) < clo.in_direction(d)) {
      if (jb == ib) {
	ce->set_direction(d, cs->in_direction(d));
	ib_found = true;
	break;
      }
      cs->set_direction(d, clo.in_direction(d));
      jb++;
    }
    if (cb.in_direction(d) > cbo.in_direction(d)) {
      if (jb == ib) {
	cs->set_direction(d, ce->in_direction(d));
	ib_found = true;
	break;
      }
      ce->set_direction(d, cbo.in_direction(d));
      jb++;
    }
  }
  if (!ib_found) { // yucky interaction here with LOOP_OVER_VOL_NOTOWNED
    *cs = one_ivec(dim);
    *ce = -one_ivec(dim);
  }
  return ib_found;
}

// first "owned" point for c in grid_volume (see also grid_volume::owns)
ivec grid_volume::little_owned_corner(component c) const {
  ivec iloc(little_owned_corner0(c));
  if (dim == Dcyl && origin.r() == 0.0 && iloc.r() == 2)
    iloc.set_direction(R, 0);
  return iloc;
}

int grid_volume::nowned(component c) const {
  int n = 1;
  ivec pt = big_corner() - little_owned_corner(c);
  LOOP_OVER_DIRECTIONS(dim, d) n *= pt.in_direction(d) / 2 + 1;
  return n;
}

bool grid_volume::owns(const ivec &p) const {
  // owns returns true if the point "owned" by this grid_volume, meaning that it
  // is the grid_volume that would timestep the point.
  const ivec o = p - io;
  if (dim == Dcyl) {
    if (origin.r() == 0.0 && o.z() > 0 && o.z() <= nz()*2 &&
        o.r() == 0) return true;
    return o.r() > 0 && o.z() > 0 &&
           o.r() <= nr()*2 && o.z() <= nz()*2;
  } else if (dim == D3) {
    return
      o.x() > 0 && o.x() <= nx()*2 &&
      o.y() > 0 && o.y() <= ny()*2 &&
      o.z() > 0 && o.z() <= nz()*2;
  } else if (dim == D2) {
    return
      o.x() > 0 && o.x() <= nx()*2 &&
      o.y() > 0 && o.y() <= ny()*2;
  } else if (dim == D1) {
    return o.z() > 0 && o.z() <= nz()*2;
  } else {
    abort("Unsupported dimension in owns.\n");
    return false;
  }
}

int grid_volume::has_boundary(boundary_side b,direction d) const {
  switch (dim) {
  case Dcyl: return d == Z || (d == R && (b == High || get_origin().r() > 0));
  case D1: return d == Z;
  case D2: return d == X || d == Y;
  case D3: return d == X || d == Y || d == Z;
  }
  return 0; // This should never be reached.
}

int grid_volume::index(component c, const ivec &p) const {
  const ivec offset = p - io - iyee_shift(c);
  int idx = 0;
  LOOP_OVER_DIRECTIONS(dim,d) idx += offset.in_direction(d)/2*stride(d);
  return idx;
}

void grid_volume::set_strides() {
  FOR_DIRECTIONS(d) the_stride[d] = 0; // Yuck yuck yuck.
  LOOP_OVER_DIRECTIONS(dim,d)
    switch(d) {
    case Z: the_stride[d] = 1; break;
    case R: the_stride[d] = nz()+1; break;
    case X: the_stride[d] = (nz()+1)*(ny() + 1); break;
    case Y: the_stride[d] = nz() + 1; break;
    case P: break; // There is no phi stride...
    case NO_DIRECTION: break; // no stride here, either
    }
}

static inline void stupidsort(int *ind, double *w, int l) {
  while (l) {
    if (fabs(w[0]) < 2e-15) {
      w[0] = w[l-1];
      ind[0] = ind[l-1];
      w[l-1] = 0.0;
      ind[l-1] = 0;
    } else {
      w += 1;
      ind += 1;
    }
    l -= 1;
  }
}

static inline void stupidsort(ivec *locs, double *w, int l) {
  while (l) {
    if (fabs(w[0]) < 2e-15) {
      w[0] = w[l-1];
      locs[0] = locs[l-1];
      w[l-1] = 0.0;
      locs[l-1] = 0;
    } else {
      w += 1;
      locs += 1;
    }
    l -= 1;
  }
}

void grid_volume::interpolate(component c, const vec &p,
                         int indices[8], double weights[8]) const {
  ivec locs[8];
  interpolate(c, p, locs, weights);
  for (int i=0;i<8&&weights[i];i++)
    if (!owns(locs[i])) weights[i] = 0.0;
  stupidsort(locs, weights, 8);
  for (int i=0;i<8&&weights[i];i++)
    indices[i] = index(c, locs[i]);
  if (!contains(p) && weights[0]) {
    printf("Error at point %g %g\n", p.r(), p.z());
    printf("Interpolated to point %d %d\n", locs[0].r(), locs[0].z());
    printf("Or in other words... %g %g\n",
           operator[](locs[0]).r(), operator[](locs[0]).z());
    printf("I %s own the interpolated point.\n",
           owns(locs[0])?"actually":"don't");
    print();
    abort("Error made in interpolation of %s--fix this bug!!!\n",
          component_name(c));
  }
  // Throw out out of range indices:
  for (int i=0;i<8&&weights[i];i++)
    if (indices[0] < 0 || indices[0] >= ntot()) weights[i] = 0.0;
  // Stupid very crude code to compactify arrays:
  stupidsort(indices, weights, 8);
  if (!contains(p) && weights[0]) {
    printf("Error at point %g %g\n", p.r(), p.z());
    printf("Interpolated to point %d %d\n", locs[0].r(), locs[0].z());
    print();
    abort("Error made in interpolation of %s--fix this bug!!!\n",
          component_name(c));
  }
}

void grid_volume::interpolate(component c, const vec &pc,
                         ivec locs[8], double weights[8]) const {
  const double SMALL = 1e-13;
  const vec p = (pc - yee_shift(c))*a;
  ivec middle(dim);
  LOOP_OVER_DIRECTIONS(dim,d)
    middle.set_direction(d, ((int) floor(p.in_direction(d)))*2+1);
  middle += iyee_shift(c);
  const vec midv = operator[](middle);
  const vec dv = (pc - midv)*(2*a);
  int already_have = 1;
  for (int i=0;i<8;i++) {
    locs[i] = round_vec(midv);
    weights[i] = 1.0;
  }
  LOOP_OVER_DIRECTIONS(dim,d) {
    for (int i=0;i<already_have;i++) {
      locs[already_have+i] = locs[i];
      weights[already_have+i] = weights[i];
      locs[i].set_direction(d,middle.in_direction(d)-1);
      weights[i] *= 0.5*(1.0-dv.in_direction(d));
      locs[already_have+i].set_direction(d,middle.in_direction(d)+1);
      weights[already_have+i] *= 0.5*(1.0+dv.in_direction(d));
    }
    already_have *= 2;
  }
  for (int i=already_have;i<8;i++) weights[i] = 0.0;
  double total_weight = 0.0;
  for (int i=0;i<already_have;i++) total_weight += weights[i];
  for (int i=0;i<already_have;i++)
    weights[i] += (1.0 - total_weight)*(1.0/already_have);
  for (int i=0;i<already_have;i++) {
    if (weights[i] < 0.0) {
      if (-weights[i] >= SMALL * 1e5)
        abort("large negative interpolation weight[%d] = %e\n", i, weights[i]);
      weights[i] = 0.0;
    }
    else if (weights[i] < SMALL)
      weights[i] = 0.0;
  }
  stupidsort(locs, weights, already_have);
  // The rest of this code is a crude hack to get the weights right when we
  // are exactly between a few grid points.  i.e. to eliminate roundoff
  // error.
  bool all_same = true;
  for (int i=0;i<8&&weights[i];i++)
    if (weights[i] != weights[0]) all_same = false;
  if (all_same) {
    int num_weights = 0;
    for (int i=0;i<8&&weights[i];i++) num_weights++;
    for (int i=0;i<8&&weights[i];i++) weights[i] = 1.0/num_weights;    
  }
}

volume empty_volume(ndim dim) {
  volume out(dim);
  LOOP_OVER_DIRECTIONS(dim,d) {
    out.set_direction_max(d,0.0);
    out.set_direction_min(d,0.0);
  }
  return out;
}

volume grid_volume::dV(const ivec &here, double diameter) const {
  const double hinva = 0.5*inva * diameter;
  const grid_volume &gv = *this;
  const vec h = gv[here];
  volume out(dim);
  LOOP_OVER_DIRECTIONS(dim,d) {
    out.set_direction_max(d,h.in_direction(d)+hinva);
    out.set_direction_min(d,h.in_direction(d)-hinva);
  }
  if (dim == Dcyl && here.r() == 0) {
    out.set_direction_min(R,0.0);
  }
  return out;
}

volume grid_volume::dV(component c, int ind) const {
  if (!owns(iloc(c, ind))) return empty_volume(dim);
  return dV(iloc(c,ind));
}

double grid_volume::xmax() const {
  const double qinva = 0.25*inva;
  return origin.x() + nx()*inva + qinva;
}

double grid_volume::xmin() const {
  const double qinva = 0.25*inva;
  return origin.x() + qinva;
}

double grid_volume::ymax() const {
  const double qinva = 0.25*inva;
  return origin.y() + ny()*inva + qinva;
}

double grid_volume::ymin() const {
  const double qinva = 0.25*inva;
  return origin.y() + qinva;
}

double grid_volume::zmax() const {
  const double qinva = 0.25*inva;
  return origin.z() + nz()*inva + qinva;
}

double grid_volume::zmin() const {
  const double qinva = 0.25*inva;
  return origin.z() + qinva;
}

double grid_volume::rmax() const {
  const double qinva = 0.25*inva;
  if (dim == Dcyl) return origin.r() + nr()*inva + qinva;
  abort("No rmax in these dimensions.\n");
  return 0.0; // This is never reached.
}

double grid_volume::rmin() const {
  const double qinva = 0.25*inva;
  if (dim == Dcyl) {
    if (origin.r() == 0.0) {
      return 0.0;
    } else {
      return origin.r() + qinva;
    }
  }
  abort("No rmin in these dimensions.\n");
  return 0.0; // This is never reached.
}

double vec::project_to_boundary(direction d, double boundary_loc) {
  return fabs(boundary_loc - in_direction(d));
}

double grid_volume::boundary_location(boundary_side b, direction d) const {
  // Returns the location of metallic walls...
  if (b == High) switch (d) {
  case X: return loc(Ez,ntot()-1).x();
  case Y: return loc(Ez,ntot()-1).y();
  case R: return loc(Ep,ntot()-1).r();
  case Z: if (dim == Dcyl) return loc(Ep,ntot()-1).z();
          else return loc(Ex,ntot()-1).z();
  case P: abort("P has no boundary!\n");
  case NO_DIRECTION: abort("NO_DIRECTION has no boundary!\n");
  }
  else switch (d) {
  case X: return loc(Ez,0).x();
  case Y: return loc(Ez,0).y();
  case R: return loc(Ep,0).r();
  case Z: if (dim == Dcyl) return loc(Ep,0).z();
          else return loc(Ex,0).z();
  case P: abort("P has no boundary!\n");
  case NO_DIRECTION: abort("NO_DIRECTION has no boundary!\n");
  }
  return 0.0;
}

ivec grid_volume::big_corner() const {
  switch (dim) {
  case D1: return io + ivec(nz())*2;
  case D2: return io + ivec(nx(),ny())*2;
  case D3: return io + ivec(nx(),ny(),nz())*2;
  case Dcyl: return io + iveccyl(nr(),nz())*2;
  }
  return ivec(0); // This is never reached.
}

vec grid_volume::corner(boundary_side b) const { 
  if (b == Low) return origin; // Low corner
  vec tmp = origin;
  LOOP_OVER_DIRECTIONS(dim, d)
    tmp.set_direction(d, tmp.in_direction(d) + num_direction(d) * inva);
  return tmp; // High corner
}

void grid_volume::print() const {
  LOOP_OVER_DIRECTIONS(dim, d)
    printf("%s =%5g - %5g (%5g) \t", 
      direction_name(d), origin.in_direction(d), 
      origin.in_direction(d)+num_direction(d)/a, num_direction(d)/a); 
  printf("\n");
}

bool grid_volume::intersect_with(const grid_volume &vol_in, grid_volume *intersection, grid_volume *others, int *num_others) const {
  int temp_num[3] = {0,0,0};
  ivec new_io(dim);
  LOOP_OVER_DIRECTIONS(dim, d) {
    int minval = max(little_corner().in_direction(d), vol_in.little_corner().in_direction(d));
    int maxval = min(big_corner().in_direction(d), vol_in.big_corner().in_direction(d));
    if (minval >= maxval)
      return false;
    temp_num[d%3] = (maxval - minval)/2;
    new_io.set_direction(d, minval);
  }
  if (intersection != NULL) {
    *intersection = grid_volume(dim, a, temp_num[0], temp_num[1], temp_num[2]); // fix me : ugly, need new constructor
    intersection->set_origin(new_io);
  }
  if (others != NULL) {
    int counter = 0;
    grid_volume vol_containing = *this;
    LOOP_OVER_DIRECTIONS(dim, d) {
      if (vol_containing.little_corner().in_direction(d)
	  < vol_in.little_corner().in_direction(d)) {
	// shave off lower slice from vol_containing and add it to others
	grid_volume other = vol_containing;
	const int thick = (vol_in.little_corner().in_direction(d)
			   - vol_containing.little_corner().in_direction(d))/2;
	other.set_num_direction(d, thick);
	others[counter] = other;
	counter++;
	vol_containing.shift_origin(d, thick*2);
	vol_containing.set_num_direction(d, vol_containing.num_direction(d)
					 - thick);
	if (vol_containing.little_corner().in_direction(d)
	    < vol_in.little_corner().in_direction(d))
	  abort("intersect_with: little corners differ by odd integer?");
      }
      if (vol_containing.big_corner().in_direction(d)
	  > vol_in.big_corner().in_direction(d)) {
	// shave off upper slice from vol_containing and add it to others
	grid_volume other = vol_containing;
	const int thick = (vol_containing.big_corner().in_direction(d)
			   - vol_in.big_corner().in_direction(d))/2;
	other.set_num_direction(d, thick);
	other.shift_origin(d, (vol_containing.num_direction(d) - thick)*2);
	others[counter] = other;
	counter++;
	vol_containing.set_num_direction(d, vol_containing.num_direction(d) 
					 - thick);
	if (vol_containing.big_corner().in_direction(d)
	    < vol_in.big_corner().in_direction(d))
	  abort("intersect_with: big corners differ by odd integer?");
      }
    }
    *num_others = counter;
    
    int initial_points = 1;
    LOOP_OVER_DIRECTIONS(dim, d) initial_points *= num_direction(d);
    int final_points , temp = 1;
    LOOP_OVER_DIRECTIONS(dim, d) temp *= intersection->num_direction(d);    
    final_points = temp;
    for (int j=0; j<*num_others; j++) {
      temp = 1;
      LOOP_OVER_DIRECTIONS(dim, d) temp *= others[j].num_direction(d);
      final_points += temp;
    }
    if (initial_points != final_points)
      abort("intersect_with: initial_points != final_points,  %d, %d\n", 
	    initial_points, final_points);
  }
  return true;
}

vec grid_volume::loc_at_resolution(int index, double res) const {
  vec where = origin;
  for (int dd=X;dd<=R;dd++) {
    const direction d = (direction) dd;
    if (has_boundary(High,d)) {
      const double dist = boundary_location(High,d)-boundary_location(Low,d);
      const int nhere = max(1,(int)floor(dist*res+0.5));
      where.set_direction(d,origin.in_direction(d) +
                          ((index % nhere)+0.5)*(1.0/res));
      index /= nhere;
    }
  }
  return where;
}

int grid_volume::ntot_at_resolution(double res) const {
  int mytot = 1;
  for (int d=X;d<=R;d++)
    if (has_boundary(High,(direction)d)) {
      const double dist = boundary_location(High,(direction)d)
                        - boundary_location(Low,(direction)d);
      mytot *= max(1,(int)(dist*res+0.5));
    }
  return mytot;
}

vec grid_volume::loc(component c, int ind) const {
  return operator[](iloc(c,ind));
}

ivec grid_volume::iloc(component c, int ind) const {
  ivec out(dim);
  LOOP_OVER_DIRECTIONS(dim,d) {
    int ind_over_stride = ind/stride(d);
    while (ind_over_stride < 0) ind_over_stride += num_direction(d)+1;
    out.set_direction(d, 2*(ind_over_stride%(num_direction(d)+1)));
  }
  return out + iyee_shift(c) + io;
}

vec grid_volume::dr() const {
  switch (dim) {
  case Dcyl: return veccyl(inva, 0.0);
  case D1: case D2: case D3: abort("Error in dr\n");
  }
  return vec(0); // This is never reached.
}

vec grid_volume::dx() const {
  switch (dim) {
  case D3: return vec(inva,0,0);
  case D2: return vec(inva,0);
  case D1: case Dcyl: abort("Error in dx.\n");
  }
  return vec(0); // This is never reached.
}

vec grid_volume::dy() const {
  switch (dim) {
  case D3: return vec(0,inva,0);
  case D2: return vec(0,inva);
  case D1: case Dcyl: abort("Error in dy.\n");
  }
  return vec(0); // This is never reached.
}

vec grid_volume::dz() const {
  switch (dim) {
  case Dcyl: return veccyl(0.0,inva);
  case D3: return vec(0,0,inva);
  case D1: return vec(inva);
  case D2: abort("dz doesn't exist in 2D\n");
  }
  return vec(0); // This is never reached.
}

grid_volume volone(double zsize, double a) {
  return grid_volume(D1, a, 0, 0, (int) (zsize*a + 0.5));
}

grid_volume voltwo(double xsize, double ysize, double a) {
  return grid_volume(D2, a, (xsize==0)?1:(int) (xsize*a + 0.5),
                       (ysize==0)?1:(int) (ysize*a + 0.5),0);
}

grid_volume vol1d(double zsize, double a) {
  return volone(zsize, a);
}

grid_volume vol2d(double xsize, double ysize, double a) {
  return voltwo(xsize, ysize, a);
}

grid_volume vol3d(double xsize, double ysize, double zsize, double a) {
  return grid_volume(D3, a,(xsize==0)?1:(int) (xsize*a + 0.5),
                      (ysize==0)?1:(int) (ysize*a + 0.5),
                      (zsize==0)?1:(int) (zsize*a + 0.5));
}

grid_volume volcyl(double rsize, double zsize, double a) {
  if (zsize == 0.0) return grid_volume(Dcyl, a, (int) (rsize*a + 0.5), 0, 1);
  else return grid_volume(Dcyl, a, (int) (rsize*a + 0.5), 0, (int) (zsize*a + 0.5));
}

grid_volume grid_volume::split(int n, int which) const {
  if (n > nowned_min())
    abort("Cannot split %d grid points into %d parts\n", nowned_min(), n);
  if (n == 1) return *this;

  // Try to get as close as we can...
  int biglen = 0;
  for (int i=0;i<3;i++) if (num[i] > biglen) biglen = num[i];
  const int split_point = (int)(biglen*(n/2)/(double)n + 0.5);
  const int num_low = (int)(split_point*n/(double)biglen + 0.5);
  if (which < num_low)
    return split_at_fraction(false, split_point).split(num_low,which);
  else
    return split_at_fraction(true, split_point).split(n-num_low,which-num_low);
}

grid_volume grid_volume::split_by_effort(int n, int which, int Ngv, const grid_volume *v, double *effort) const {
  const int grid_points_owned = nowned_min();
  if (n > grid_points_owned)
    abort("Cannot split %d grid points into %d parts\n", nowned_min(), n);
  if (n == 1) return *this;
  int biglen = 0;
  direction splitdir = NO_DIRECTION;
  LOOP_OVER_DIRECTIONS(dim, d) if (num_direction(d) > biglen) { biglen = num_direction(d); splitdir = d; } 
  double best_split_measure = 1e20, left_effort_fraction = 0;
  int best_split_point = 0;
  vec corner = zero_vec(dim);
  LOOP_OVER_DIRECTIONS(dim, d) corner.set_direction(d, origin.in_direction(d) + num_direction(d)/a); 

  for (int split_point = 1; split_point < biglen; split_point+=1) {
    grid_volume v_left = *this;
    v_left.set_num_direction(splitdir, split_point);
    grid_volume v_right = *this;
    v_right.set_num_direction(splitdir, num_direction(splitdir) - split_point);
    v_right.shift_origin(splitdir, split_point*2);

    double total_left_effort = 0, total_right_effort = 0;
    grid_volume vol;
    if (Ngv == 0) {
      total_left_effort = v_left.ntot();
      total_right_effort = v_right.ntot();
    }
    else {
      for (int j = 0; j<Ngv; j++) {
	if (v_left.intersect_with(v[j], &vol))
	  total_left_effort += effort[j] * vol.ntot();
	if (v_right.intersect_with(v[j], &vol))
	  total_right_effort += effort[j] * vol.ntot();
      }
    }
    double split_measure = max(total_left_effort/(n/2), total_right_effort/(n-n/2));
    if (split_measure < best_split_measure) {
      best_split_measure = split_measure;
      best_split_point = split_point;
      left_effort_fraction = total_left_effort/(total_left_effort + total_right_effort);
    }
  }
  const int split_point = best_split_point;
    
  const int num_low = (int)(left_effort_fraction *n + 0.5);
  // Revert to split() when effort method gives less grid points than chunks
  if (num_low > best_split_point*(grid_points_owned/biglen) || 
      (n-num_low) > (grid_points_owned - best_split_point*(grid_points_owned/biglen)))
    return split(n, which);

  if (which < num_low)
    return split_at_fraction(false, split_point).split_by_effort(num_low,which, Ngv,v,effort);
  else
    return split_at_fraction(true, split_point).split_by_effort(n-num_low,which-num_low, Ngv,v,effort);
}

grid_volume grid_volume::split_at_fraction(bool want_high, int numer) const {
  int bestd = -1, bestlen = 1;
  for (int i=0;i<3;i++)
    if (num[i] > bestlen) {
      bestd = i;
      bestlen = num[i];
    }
  if (bestd == -1) {
    for (int i=0;i<3;i++) master_printf("num[%d] = %d\n", i, num[i]);
    abort("Crazy weird splitting error.\n");
  }
  grid_volume retval(dim, a, 1,1,1);
  for (int i=0;i<3;i++) retval.num[i] = num[i];
  if (numer >= num[bestd])
    abort("Aaack bad bug in split_at_fraction.\n");
  direction d = (direction) bestd;
  if (dim == Dcyl && d == X) d = R;
  retval.set_origin(io);
  if (want_high)
    retval.shift_origin(d,numer*2);

  if (want_high) retval.num[bestd] -= numer;
  else retval.num[bestd] = numer;
  retval.num_changed();
  return retval;
}

// Halve the grid_volume for symmetry exploitation...must contain icenter!
grid_volume grid_volume::halve(direction d) const {
  grid_volume retval(*this);
  // note that icenter-io is always even by construction of grid_volume::icenter
  retval.set_num_direction(d, (icenter().in_direction(d) 
			       - io.in_direction(d)) / 2);
  return retval;
}

grid_volume grid_volume::pad(direction d) const {
  grid_volume gv(*this);
  gv.pad_self(d);
  return gv;
}

void grid_volume::pad_self(direction d) {
  num[d%3]+=2; // Pad in both directions by one grid point.
  num_changed();
  shift_origin(d, -2);
}

ivec grid_volume::icenter() const {
  /* Find the center of the user's cell.  This will be used as the
     symmetry point, and therefore icenter-io must be *even*
     in all components in order that rotations preserve the Yee lattice. */
  switch (dim) {
  case D1: return io + ivec(nz()).round_up_to_even();
  case D2: return io + ivec(nx(), ny()).round_up_to_even();
  case D3: return io + ivec(nx(), ny(), nz()).round_up_to_even();
  case Dcyl: return io + iveccyl(0, nz()).round_up_to_even();
  }
  abort("Can't do symmetry with these dimensions.\n");
  return ivec(0); // This is never reached.
}

vec grid_volume::center() const {
  return operator[](icenter());
}

symmetry rotate4(direction axis, const grid_volume &gv) {
  symmetry s = identity();
  if (axis > 2) abort("Can only rotate4 in 2D or 3D.\n");
  s.g = 4;
  FOR_DIRECTIONS(d) {
    s.S[d].d = d;
    s.S[d].flipped = false;
  }
  s.S[(axis+1)%3].d = (direction)((axis+2)%3);
  s.S[(axis+1)%3].flipped = true;
  s.S[(axis+2)%3].d = (direction)((axis+1)%3);
  s.symmetry_point = gv.center();
  s.i_symmetry_point = gv.icenter();
  return s;
}

symmetry rotate2(direction axis, const grid_volume &gv) {
  symmetry s = identity();
  if (axis > 2) abort("Can only rotate2 in 2D or 3D.\n");
  s.g = 2;
  s.S[(axis+1)%3].flipped = true;
  s.S[(axis+2)%3].flipped = true;
  s.symmetry_point = gv.center();
  s.i_symmetry_point = gv.icenter();
  return s;
}

symmetry mirror(direction axis, const grid_volume &gv) {
  symmetry s = identity();
  s.g = 2;
  s.S[axis].flipped = true;
  s.symmetry_point = gv.center();
  s.i_symmetry_point = gv.icenter();
  return s;
}

symmetry r_to_minus_r_symmetry(double m) {
  symmetry s = identity();
  s.g = 2;
  s.S[R].flipped = true;
  s.S[P].flipped = true;
  s.symmetry_point = zero_vec(Dcyl);
  s.i_symmetry_point = zero_ivec(Dcyl);
  if (m == int(m)) // phase is purely real (+/- 1) when m an integer
    s.ph = (int(m) & 1) ? -1.0 : 1.0;
  else
    s.ph = polar(1.0, m * pi); // general case
  return s;
}

symmetry identity() {
  return symmetry();
}

symmetry::symmetry() {
  g = 1;
  ph = 1.0;
  FOR_DIRECTIONS(d) {
    S[d].d = d;
    S[d].flipped = false;
  }
  next = NULL;
}

symmetry::symmetry(const symmetry &s) {
  g = s.g;
  FOR_DIRECTIONS(d) {
    S[d].d = s.S[d].d;
    S[d].flipped = s.S[d].flipped;
  }
  ph = s.ph;
  symmetry_point = s.symmetry_point;
  i_symmetry_point = s.i_symmetry_point;
  if (s.next) next = new symmetry(*s.next);
  else next = NULL;
}

void symmetry::operator=(const symmetry &s) {
  g = s.g;
  FOR_DIRECTIONS(d) {
    S[d].d = s.S[d].d;
    S[d].flipped = s.S[d].flipped;
  }
  ph = s.ph;
  symmetry_point = s.symmetry_point;
  i_symmetry_point = s.i_symmetry_point;
  if (s.next) next = new symmetry(*s.next);
  else next = NULL;
}

bool symmetry::operator==(const symmetry &sym) const {
  int gtot = multiplicity();
  if (gtot != sym.multiplicity())
    return false;
  for (int sn = 1; sn < gtot; ++sn)
    FOR_DIRECTIONS(d)
      if (transform(d, sn) != sym.transform(d, sn))
	return false;
  return true;
}

symmetry::~symmetry() {
  delete next;
}

int symmetry::multiplicity() const {
  if (next) return g*next->multiplicity();
  else return g;
}

symmetry symmetry::operator+(const symmetry &b) const {
  // The following optimization ignores identity when adding symmetries
  // together.  This is important because identity has an undefined
  // symmetry point.
  if (multiplicity() == 1) return b;
  else if (b.multiplicity() == 1) return *this;
  symmetry s = *this;
  symmetry *sn = &s;
  for (; sn->next; sn = sn->next) ;
  sn->next = new symmetry(b);
  return s;
}

symmetry symmetry::operator*(complex<double> p) const {
  symmetry s = *this;
  s.ph *= p;
  return s;
}

signed_direction signed_direction::operator*(complex<double> p) {
  signed_direction sd = *this;
  sd.phase *= p;
  return sd;
}

signed_direction symmetry::transform(direction d, int n) const {
  // Returns transformed direction + phase/flip; -n indicates inverse transform
  if (n == 0 || d == NO_DIRECTION) return signed_direction(d);
  int nme, nrest;
  if (n < 0) {
       nme = (g - (-n) % g) % g;
       nrest = -((-n) / g);
  } else {
       nme = n % g;
       nrest = n / g;
  }
  if (nme == 0) {
    if (nrest == 0) return signed_direction(d);
    else return next->transform(d,nrest);
  } else {
    signed_direction sd;
    if (nme == 1) sd = S[d];
    if (S[d].flipped) sd = flip(transform(S[d].d, nme-1));
    else sd = transform(S[d].d, nme-1);

    if (next && nrest) {
      if (sd.flipped) return flip(next->transform(sd.d, nrest))*ph;
      else return next->transform(sd.d, nrest)*ph;
    } else {
      return sd*ph;
    }
  }
}

ivec symmetry::transform(const ivec &ov, int n) const {
  if (n == 0) return ov;
  ivec out = ov;
  LOOP_OVER_DIRECTIONS(ov.dim, d) {
    const signed_direction s = transform(d,n);
    const int sp_d  = i_symmetry_point.in_direction(d);
    const int sp_sd = i_symmetry_point.in_direction(s.d);
    const int delta = ov.in_direction(d) - sp_d;
    if (s.flipped) out.set_direction(s.d, sp_sd - delta);
    else out.set_direction(s.d, sp_sd + delta);
  }
  return out;
}

ivec symmetry::transform_unshifted(const ivec &ov, int n) const {
  if (n == 0) return ov;
  ivec out(ov.dim);
  LOOP_OVER_DIRECTIONS(ov.dim, d) {
    const signed_direction s = transform(d,n);
    if (s.flipped) out.set_direction(s.d, -ov.in_direction(d));
    else out.set_direction(s.d, ov.in_direction(d));
  }
  return out;
}

vec symmetry::transform(const vec &ov, int n) const {
  if (n == 0) return ov;
  vec delta = ov;
  LOOP_OVER_DIRECTIONS(ov.dim, d) {
    const signed_direction s = transform(d,n);
    double deltad = ov.in_direction(d) - symmetry_point.in_direction(d);
    if (s.flipped) delta.set_direction(s.d, -deltad);
    else delta.set_direction(s.d, deltad);
  }
  return symmetry_point + delta;
}

volume symmetry::transform(const volume &v, int n) const {
  return volume(transform(v.get_min_corner(),n),
                          transform(v.get_max_corner(),n));
}

component symmetry::transform(component c, int n) const {
  return direction_component(c,transform(component_direction(c),n).d);
}

derived_component symmetry::transform(derived_component c, int n) const {
  return direction_component(c,transform(component_direction(c),n).d);
}

int symmetry::transform(int c, int n) const {
  return (is_derived(c) ? int(transform(derived_component(c), n))
	  : int(transform(component(c), n)));
}

complex<double> symmetry::phase_shift(component c, int n) const {
  if (c == Dielectric || c == Permeability) return 1.0;
  complex<double> phase = transform(component_direction(c),n).phase;
  // flip tells us if we need to flip the sign.  For vectors (E), it is
  // just this simple:
  bool flip = transform(component_direction(c),n).flipped;
  if (is_magnetic(c) || is_B(c)) {
    // Because H is a pseudovector, here we have to figure out if the
    // transformation changes the handedness of the basis.
    bool have_one = false, have_two = false;
    FOR_DIRECTIONS(d) {
      if (transform(d,n).flipped) flip = !flip;
      int shift = (transform(d,n).d - d + 6) % 3;
      if (shift == 1) have_one = true;
      if (shift == 2) have_two = true;
    }
    if (have_one && have_two) flip = !flip;
  }
  if (flip) return -phase;
  else return phase;
}

complex<double> symmetry::phase_shift(derived_component c, int n) const {
  if (is_poynting(c)) {
    signed_direction ds = transform(component_direction(c),n);
    complex<double> ph = conj(ds.phase) * ds.phase; // E x H gets |phase|^2
    return (ds.flipped ? -ph : ph);
  }
  else /* energy density */
    return 1.0;
}

complex<double> symmetry::phase_shift(int c, int n) const {
  return (is_derived(c) ? phase_shift(derived_component(c), n)
	  : phase_shift(component(c), n));
}

bool symmetry::is_primitive(const ivec &p) const {
  // This is only correct if p is somewhere on the yee lattice.
  if (multiplicity() == 1) return true;
  for (int i=1;i<multiplicity();i++) {
    const ivec pp = transform(p,i);
    switch (p.dim) {
    case D2:
      if (pp.x()+pp.y() < p.x()+p.y()) return false;
      if (pp.x()+pp.y() == p.x()+p.y() &&
          p.y() > p.x() && pp.y() <= pp.x()) return false;
      break;
    case D3:
      if (pp.x()+pp.y()+pp.z() <  p.x()+p.y()+p.z()) return false;
      if (pp.x()+pp.y()+pp.z() == p.x()+p.y()+p.z() &&
          pp.x()+pp.y()-pp.z() <  p.x()+p.y()-p.z()) return false;
      if (pp.x()+pp.y()+pp.z() == p.x()+p.y()+p.z() &&
          pp.x()+pp.y()-pp.z() == p.x()+p.y()-p.z() &&
          pp.x()-pp.y()-pp.z() <  p.x()-p.y()-p.z()) return false;
      break;
    case D1: case Dcyl:
      if (pp.z() < p.z()) return false;
      break;
    }
  }
  return true;
}

/* given a list of geometric volumes, produce a new list with appropriate
   weights that is minimized according to the symmetry.  */
volume_list *symmetry::reduce(const volume_list *gl) const {
  volume_list *glnew = 0;
  for (const volume_list *g = gl; g; g = g->next) {
    int sn;
    for (sn = 0; sn < multiplicity(); ++sn) {
      volume gS(transform(g->v, sn));
      int cS = transform(g->c, sn);
      volume_list *gn;
      for (gn = glnew; gn; gn = gn->next)
	if (gn->c == cS && gn->v.round_float() == gS.round_float())
	  break;
      if (gn) { // found a match
	gn->weight += g->weight * phase_shift(g->c, sn);
	break;
      }
    }
    if (sn == multiplicity() && g->weight != 0.0) { // no match, add to glnew
      volume_list *gn = 
	new volume_list(g->v, g->c, g->weight, glnew);
      glnew = gn;
    }
  }

  // reduce v's redundant with themselves & delete elements with zero weight:
  volume_list *gprev = 0, *g = glnew;
  while (g) {
    // first, see if g->v is redundant with itself
    bool halve[5] = {false,false,false,false,false};
    complex<double> weight = g->weight;
    for (int sn = 1; sn < multiplicity(); ++sn)
      if (g->c == transform(g->c, sn) && 
	  g->v.round_float() == transform(g->v, sn).round_float()) {
	LOOP_OVER_DIRECTIONS(g->v.dim, d)
	  if (transform(d,sn).flipped) {
	    halve[d] = true;
	    break;
	  }
	g->weight += weight * phase_shift(g->c, sn);
      }
    LOOP_OVER_DIRECTIONS(g->v.dim, d)
      if (halve[d])
	g->v.set_direction_max(d, g->v.in_direction_min(d) +
				0.5 * g->v.in_direction(d));
    
      // now, delete it if it has zero weight
    if (g->weight == 0.0) {
      if (gprev)
	gprev->next = g->next;
      else // g == glnew
	glnew = g->next;
      g->next = 0; // necessary so that g->next is not deleted recursively
      delete g;
      g = gprev ? gprev->next : glnew;
    }
    else
      g = (gprev = g)->next;
  }

  return glnew;
}

/***************************************************************************/

static double poynting_fun(const complex<double> *fields,
		       const vec &loc, void *data_)
{
     (void) loc; // unused
     (void) data_; // unused
     return (real(conj(fields[0]) * fields[1])
	     - real(conj(fields[2])*fields[3]));
}

static double energy_fun(const complex<double> *fields,
		       const vec &loc, void *data_)
{
     (void) loc; // unused
     int nfields = *((int *) data_) / 2;
     double sum = 0;
     for (int k = 0; k < nfields; ++k)
       sum += real(conj(fields[2*k]) * fields[2*k+1]);
     return sum * 0.5;
}

field_rfunction derived_component_func(derived_component c, const grid_volume &gv,
				       int &nfields, component cs[12]) {
  switch (c) {
  case Sx: case Sy: case Sz: case Sr: case Sp:
    switch (c) {
    case Sx: cs[0] = Ey; cs[1] = Hz; break;
    case Sy: cs[0] = Ez; cs[1] = Hx; break;
    case Sz: cs[0] = Ex; cs[1] = Hy; break;
    case Sr: cs[0] = Ep; cs[1] = Hz; break;
    case Sp: cs[0] = Ez; cs[1] = Hr; break;
    default: break; // never reached
    }
    nfields = 4;
    cs[2] = direction_component(Ex, component_direction(cs[1]));
    cs[3] = direction_component(Hx, component_direction(cs[0]));
    return poynting_fun;

  case EnergyDensity: case D_EnergyDensity: case H_EnergyDensity:
    nfields = 0;
    if (c != H_EnergyDensity)
      FOR_ELECTRIC_COMPONENTS(c0) if (gv.has_field(c0)) {
	cs[nfields++] = c0;
	cs[nfields++] = direction_component(Dx, component_direction(c0));
      }
    if (c != D_EnergyDensity)
      FOR_MAGNETIC_COMPONENTS(c0) if (gv.has_field(c0)) {
	cs[nfields++] = c0;
	cs[nfields++] = direction_component(Bx, component_direction(c0));
      }
    if (nfields > 12) abort("too many field components");
    return energy_fun;

  default: 
    abort("unknown derived_component in derived_component_func");
  }
  return 0;
}

/*	snapshot.cpp
 *
 *  v0.1.0 August 2010 - Arthur Thijssen (thijssen.arthur@gmail.com)
 *
 *  v0.1.1 February 2011 - Arthur Thijssen (thijssen.arthur@gmail.com)
 *		- Using SWIG wrappers so full integration with MEEP is possible while still using the libctl interface
 *
 *	v0.1.2 March 2011 - Arthur Thijssen (thijssen.arthur@gmail.com)
 *		- Included a near to far field transform
 *
 *  v0.2.0 May 2011	- Arthur Thijssen (thijssen.arthur@gmail.com)
 *		- Included an integrated frequency dependant modal volume calculation
 *
 *  v0.2.1 June 2011 - Arthur Thijssen (thijssen.arthur@gmail.com)
 *		- When outputting large datasets in MPI some cores deadlocked for unexplicable reasons.
 *		  Considering all data was written using the master anyway (passing data to 1 core looks faster then an MPI hdf5 output)
 *        all file handling has been moved to the master.
 *
 *	v0.2.2 March 2014 - Arthur Thijssen (thijssen.arthur@gmail.com)
 *		- Updated code for Meep 1.2
 *
 */

snapshot:: snapshot( fields * f, int n_comp, const char * name, const vec &center, const vec &size, double r, direction dir, double l, double res )
{
	f->am_now_working_on( SnapCreate );

	_f			= f;
	radius		= r;
	d			= dir;
	freq		= l;
	resolution  = res;						// Should not exceed the meep resolution

	_data_mag = NULL;
	_data_arg = NULL;

	n_dims[ 0 ] = 1;
	n_dims[ 1 ] = 1;
	n_dims[ 2 ] = 1;
	n_car[ 0 ] = 1;
	n_car[ 1 ] = 1;
	n_car[ 2 ] = 1;

	if ( f->v.dim == D1 )
		{	// 1D
		_center		= new vec( 0.0, 0.0, center.z() );
		_size		= new vec( 0.0, 0.0, size.z() );
		n_car[ 2 ] = (int) ceil( _size->z() * resolution + 1.0 );
		}
	else if ( f->v.dim == D2 )
		{	// 2D
		_center		= new vec( center.x(), center.y(), 0.0 );
		_size		= new vec( size.x(), size.y(), 0.0 );
		n_car[ 0 ] = (int) ceil( _size->x() * resolution + 1.0 );
		n_car[ 1 ] = (int) ceil( _size->y() * resolution + 1.0 );
		}
	else if ( f->v.dim == Dcyl )
        {
		_center		= new vec( center.r(), 0, center.z() );
		_size		= new vec( size.r(), 0, size.z() );
		n_car[ 0 ] = (int) ceil( _size->x() * resolution + 1.0 );
		n_car[ 1 ] = (int) 1.0;
		n_car[ 2 ] = (int) ceil( _size->z() * resolution + 1.0 );
        }
    else
		{	// 3D
		_center		= new vec( center.x(), center.y(), center.z() );
		_size		= new vec( size.x(), size.y(), size.z() );
		n_car[ 0 ] = (int) ceil( _size->x() * resolution + 1.0 );
		n_car[ 1 ] = (int) ceil( _size->y() * resolution + 1.0 );
		n_car[ 2 ] = (int) ceil( _size->z() * resolution + 1.0 );
		}

	_name		= new char[ strlen( name ) + 1 ];
	strcpy( _name, name);

	_c			= new component [ n_comp ];
	n_c			= n_comp;
	for ( int n = 0 ; n < n_c ; n++ )
		{
		_c[ n ] = Ex;
		}

	if ( radius == 0 )
		{
		rank = 0;
		for ( int n = 0 ; n < 3 ; n++ )
			{
			if ( n_car[ n ] > 1 )
				{
				n_dims[ rank ] = n_car[ n ];
				rank++;
				}
			}
		}
	else
		{
		n_dims[ 0 ] = (int) ceil( pi * resolution / sqrt( 2.0 ) );
		n_dims[ 1 ] = (int) ceil( (float) n_dims[ 0 ] / 2.0 );
		rank = 2;
		}
	_dft_chunk_array_ptr = allocate_memory();
	_f->finished_working();
}

snapshot:: ~snapshot()
{
	for ( int comp = 0 ; comp < n_c ; comp++ )
		{
		for ( int n_0 = 0 ; n_0 < n_dims[ 0 ] ; n_0++ )
			{
			for ( int n_1 = 0 ; n_1 < n_dims[ 1 ] ; n_1++ )
				{
				delete _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ];
				}
			delete _dft_chunk_array_ptr[ comp ][ n_0 ];
			}
		delete _dft_chunk_array_ptr[ comp ];
		}
	delete _dft_chunk_array_ptr;

	if ( _data_mag )
		{
		delete _data_mag;
		_data_mag = NULL;
		}
	if ( _data_arg )
		{
		delete _data_arg;
		_data_arg = NULL;
		}

	delete _center;
	delete _size;
	delete _name;
	delete _c;
}

void snapshot:: pass_data()
{
	complex<double> data_c;
	complex<double> data_c_buf;

	if ( am_master() )
		{
		_data_mag = new realnum *[ n_c ];
		_data_arg = new realnum *[ n_c ];
		}

	int proc_rank = my_rank();
	int procs	  = count_processors();

	for ( int comp = 0 ; comp < n_c ; comp++ )
		{
		if ( am_master() )
			{
			_data_mag[ comp ] = new realnum[ n_dims[ 0 ] * n_dims[ 1 ] * n_dims[ 2 ] ];
			_data_arg[ comp ] = new realnum[ n_dims[ 0 ] * n_dims[ 1 ] * n_dims[ 2 ] ];
			}
		for ( int n_0 = 0 ; n_0 < n_dims[ 0 ] ; n_0++ )
			{
			for ( int n_1 = 0 ; n_1 < n_dims[ 1 ] ; n_1++ )
				{
				for ( int n_2 = 0 ; n_2 < n_dims[ 2 ] ; n_2++ )
					{
					data_c = 0.0;
					if ( _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] && _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->N > 0 )
						{
						for ( int n = 0 ; n < _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->N ; n++ )
							{
							data_c += (complex<double>) _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->dft[ n ];
							}
						data_c = data_c / (double) _dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->N;
						}
					for ( int proc = 0 ; proc < procs ; proc++ )
						{
						data_c_buf = Ssend( proc, 0, data_c );
						if ( am_master() && data_c_buf != 0.0 )
							{
							data_c = data_c_buf;
							}
						}
					if ( am_master() )
						{
						_data_mag[ comp ][ n_0 * n_dims[ 1 ] * n_dims[ 2 ] + n_1 * n_dims[ 2 ] + n_2 ] = (realnum) abs( data_c );
						_data_arg[ comp ][ n_0 * n_dims[ 1 ] * n_dims[ 2 ] + n_1 * n_dims[ 2 ] + n_2 ] = (realnum) arg( data_c );
						}
					}
				}
			}
		}
}

meep::dft_chunk ***** snapshot::allocate_memory()
{
	dft_chunk ***** temp_ptr = new dft_chunk ****[ n_c ];
	for ( int comp = 0 ; comp < n_c ; comp++ )
		{
		temp_ptr[ comp ] = new dft_chunk ***[ n_dims[ 0 ] ];
		for ( int n_0 = 0 ; n_0 < n_dims[ 0 ] ; n_0++ )
			{
			temp_ptr[ comp ][ n_0 ] = new dft_chunk **[ n_dims[ 1 ] ];
			for ( int n_1 = 0 ; n_1 < n_dims[ 1 ] ; n_1++ )
				{
				temp_ptr[ comp ][ n_0 ][ n_1 ] = new meep::dft_chunk *[ n_dims[ 2 ] ];
				}
			}
		}
	return temp_ptr;
}

void snapshot::create_dft()
{
	double x_loc, y_loc, z_loc;
	for ( int comp = 0 ; comp < n_c ; comp++ )
		{
		for ( int n_0 = 0 ; n_0 < n_dims[ 0 ] ; n_0++ )
			{
			for ( int n_1 = 0 ; n_1 < n_dims[ 1 ] ; n_1++ )
				{
				for ( int n_2 = 0 ; n_2 < n_dims[ 2 ] ; n_2++ )
					{
					x_loc = _center->x()  - _size->x() / 2.0 + ( n_car[ 0 ] != 1 || ( n_car[ 1 ] == 1 && n_car[ 2 ] == 1 ) ? ((double)n_0) : ( n_car[ 1 ] != 1 && n_car[ 2 ] != 1 ? ((double)n_2) : ((double)n_1) ) ) / resolution;
					y_loc = _center->y()  - _size->y() / 2.0 + ( n_car[ 0 ] != 1 && ( n_car[ 1 ] != 1 || n_car[ 2 ] == 1 ) ? ((double)n_1) : ( n_car[ 0 ] == 1 && ( n_car[ 1 ] != 1 || n_car[ 2 ] == 1 ) ? ((double)n_0) : ((double)n_2) ) ) / resolution;
					z_loc = _center->z()  - _size->z() / 2.0 + ( n_car[ 0 ] == 1 &&   n_car[ 1 ] == 1 && n_car[ 2 ] != 1   ? ((double)n_0) : ( n_car[ 2 ] != 1 && ( n_car[ 0 ] == 1 || n_car[ 1 ] == 1 ) ? ((double)n_1) : ((double)n_2) ) ) / resolution;
					if ( _f->v.dim == D1 )
						{	// 1D
						_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] = _f->add_dft_pt( _c[ comp ], meep::vec( z_loc ), freq, freq, 1 );
						}
					else if ( _f->v.dim == D2 )
						{	// 2D
						_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] = _f->add_dft_pt( _c[ comp ], meep::vec( x_loc, y_loc ), freq, freq, 1 );
						}
                    else if ( _f->v.dim == Dcyl )
                        {   // Cylindrical
						_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] = _f->add_dft_pt( _c[ comp ], meep::veccyl( x_loc, z_loc ), freq, freq, 1 );
                        }
					else
						{	// 3D
						_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] = _f->add_dft_pt( _c[ comp ], meep::vec( x_loc, y_loc, z_loc ), freq, freq, 1 );
						}
					}
				}
			}
		}
}

void snapshot::create_dft_sphere()
{
	double phi, theta, x_loc, y_loc, z_loc;
	for ( int comp = 0 ; comp < n_c ; comp++ )
		{
		for ( int k = 0 ; k < n_dims[ 0 ] ; k++ )
			{
			phi =  ( ( double ) k )  / ( ( double ) n_dims[ 0 ] - 1.0 ) * 2.0 * pi - pi;
			for ( int l = 0 ; l < n_dims[ 1 ] ; l++ )
				{
				theta =  ( ( double ) l ) / ( ( double ) n_dims[ 1 ] - 1.0 ) * pi / 2.0;
				if ( d == Z )
					{
					x_loc = radius * sin( theta ) * cos( phi ) + _center->x();
					y_loc = radius * sin( theta ) * sin( phi ) + _center->y();
					z_loc = radius * cos( theta ) + _center->z();
					}
				else if ( d == Y )
					{
					z_loc = radius * sin( theta ) * cos( phi ) + _center->x();
					x_loc = radius * sin( theta ) * sin( phi ) + _center->y();
					y_loc = radius * cos( theta ) + _center->z();
					}
				else
					{
					y_loc = radius * sin( theta ) * cos( phi ) + _center->x();
					z_loc = radius * sin( theta ) * sin( phi ) + _center->y();
					x_loc = radius * cos( theta ) + _center->z();
					}
				_dft_chunk_array_ptr[ comp ][ k ][ l ][ 0 ] = _f->add_dft_pt( _c[ comp ], meep::vec( x_loc, y_loc, z_loc ), freq, freq, 1 );
				}
			}
		}
}

void snapshot::output()
{
	_f->am_now_working_on( SnapComm );
	pass_data();
	_f->finished_working();
	_f->am_now_working_on( SnapOutput );
	output_snapshot();
	_f->finished_working();
}

void meep::snapshot::output_snapshot()
{
	if ( am_master() )
		{
		char * _string = new char[ strlen( _name ) + 32 ];
		strcpy( _string, _name );
		strcat( _string, ".h5\0" );
		master_printf( "creating output file \"./%s\"...\n", _string );
		if ( am_master() )
			{
			_h5file = new h5file( _string, h5file::WRITE, false );
			int start[ 3 ] = { 0, 0, 0 };
			for ( int comp = 0 ; comp < n_c ; comp++ )
				{
				sprintf( _string, "%s-mag\0", component_name( _c[ comp ] ) );
				_h5file->write( _string, rank, &n_dims[ 0 ], &_data_mag[ comp ][ 0 ], true );
				sprintf( _string, "%s-arg\0", component_name( _c[ comp ] ) );
				_h5file->write( _string, rank, &n_dims[ 0 ], &_data_arg[ comp ][ 0 ], true );
				}
			delete _h5file;
			delete _data_mag;
			delete _data_arg;
			_h5file = NULL;
			_data_mag = NULL;
			_data_arg = NULL;
			}
		delete _string;
		}
	all_wait();
}

void snapshot:: add_component( component c, int num )
{
	_c[ num ] = c;
}

void snapshot:: create()
{
	_f->am_now_working_on( SnapCreate );
	if ( radius == 0 )
		{
		create_dft();
		}
	else
		{
		create_dft_sphere();
		}
	master_printf( "Added snapshot %s\n", _name );
	_f->finished_working();
}

nf2ff:: nf2ff( fields * f, const vec &center, const vec &v_size, double l, double res, direction dir, char * name, bool output )
{
	_center		= new vec( center.x(), center.y(), center.z() );
	_size		= new vec( v_size.x(), v_size.y(), v_size.z() );
	_name		= new char[ strlen( name ) + 1 ];
	strcpy( _name, name);

	size[ 0 ]	= (int) ceil( _size->x() * res + 1.0 );
	size[ 1 ]	= (int) ceil( _size->y() * res + 1.0 );
	size[ 2 ]	= (int) ceil( _size->z() * res + 1.0 );

	_near_data				= NULL;
	_snaps					= NULL;
	_far_data_e_phi_mag		= NULL;
	_far_data_e_theta_mag	= NULL;
	_far_data_e_phi_arg		= NULL;
	_far_data_e_theta_arg	= NULL;
	_far_data_h_phi_mag		= NULL;
	_far_data_h_theta_mag	= NULL;
	_far_data_h_phi_arg		= NULL;
	_far_data_h_theta_arg	= NULL;

	_f			= f;
	freq		= l;
	resolution  = res;
	d           = dir;
	out			= output;

	res_angle[ 1 ] = (int) ceil( sqrt( (double) ( size[ 0 ] * size[ 1 ] + size[ 0 ] * size[ 2 ] + size[ 1 ] * size[ 2 ] ) ) );
	res_angle[ 0 ] = 2 * res_angle[ 1 ];

	create_snaps();
}

nf2ff:: ~nf2ff()
{
	delete _center;
	delete _size;
	delete _name;

	if ( _h5file )
		{
		delete _h5file;
		}
	if ( _far_data_e_phi_mag )
		{
		delete _far_data_e_phi_mag;
		}
	if ( _far_data_e_theta_mag )
		{
		delete _far_data_e_theta_mag;
		}
	if ( _far_data_e_phi_arg )
		{
		delete _far_data_e_phi_arg;
		}
	if ( _far_data_e_theta_arg )
		{
		delete _far_data_e_theta_arg;
		}
	if ( _far_data_h_phi_mag )
		{
		delete _far_data_h_phi_mag;
		}
	if ( _far_data_h_theta_mag )
		{
		delete _far_data_h_theta_mag;
		}
	if ( _far_data_h_phi_arg )
		{
		delete _far_data_h_phi_arg;
		}
	if ( _far_data_h_theta_arg )
		{
		delete _far_data_h_theta_arg;
		}

	_h5file					= NULL;
	_far_data_e_phi_mag		= NULL;
	_far_data_e_theta_mag	= NULL;
	_far_data_e_phi_arg		= NULL;
	_far_data_e_theta_arg	= NULL;
	_far_data_h_phi_mag		= NULL;
	_far_data_h_theta_mag	= NULL;
	_far_data_h_phi_arg		= NULL;
	_far_data_h_theta_arg	= NULL;

	if ( _near_data )
		{
		for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
			{
			if ( d == dir_index || d == NO_DIRECTION )
				{
				for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
					{
					for ( int comp = 0 ; comp < 4 ; comp++ )
						{
						for ( int x = 0 ; x < size[ dir_index == 0 ? 1 : 2 ] ; x++ )
							{
							delete _near_data[ dir_index ][ pos ][ comp ][ x ];
							}
						delete _near_data[ dir_index ][ pos ][ comp ];
						}
					delete _near_data[ dir_index ][ pos ];
					}
				delete _near_data[ dir_index ];
				}
			}
		delete _near_data;
		}
	_near_data = NULL;

	if ( _snaps )
		{
		for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
			{
			if ( d == dir_index || d == NO_DIRECTION )
				{
				for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
					{
					delete _snaps[ dir_index ][ pos ];
					}
				}
			delete _snaps[ dir_index ];
			}
		delete _snaps;
		}
	_snaps = NULL;
}

void nf2ff:: process()
{
	_f->am_now_working_on( Nf2ffCalc );
	allocate();
	_f->finished_working();
	master_printf( "Communicating nf2ff data %s\n", _name );
	_f->am_now_working_on( Nf2ffComm );
	pass_data();
	_f->finished_working();

	for ( int dir = 0 ; dir < 3 ; dir++ )
		{
		if ( d == dir || d == NO_DIRECTION )
			{
			for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
				{
				delete _snaps[ dir ][ pos ];
				}
			delete _snaps[ dir ];
			}
		}
	delete _snaps;
	_snaps = NULL;

	if ( out )
		{
		_f->am_now_working_on( SnapOutput );
		output_snaps();
		_f->finished_working();
		}

	master_printf( "Calculating nf2ff data %s\n", _name );
	_f->am_now_working_on( Nf2ffCalc );
	calculate();
	_f->finished_working();
	_f->am_now_working_on( Nf2ffOutput );
	output();
	_f->finished_working();
}

void nf2ff::calculate( )
{
	if ( am_master() )
		{
		_far_data_e_phi_mag		= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_e_theta_mag	= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_e_phi_arg		= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_e_theta_arg	= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_h_phi_mag		= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_h_theta_mag	= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_h_phi_arg		= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];
		_far_data_h_theta_arg	= new realnum[ res_angle[ 0 ] * res_angle[ 1 ] ];

		double phi, theta;
		double cost, cosp, sint, sinp;
		double sintcosp, sintsinp;
		double costsinp, costcosp;

		complex<double> C;

		complex<double> L_phi;
		complex<double> L_theta;
		complex<double> N_phi;
		complex<double> N_theta;

		complex<double> Mx;
		complex<double> My;
		complex<double> Mz;
		complex<double> Jx;
		complex<double> Jy;
		complex<double> Jz;
		double norm;

		for ( int k = 0 ; k < res_angle[ 0 ] ; k++ )
			{
			phi =  ((double) k)  / ( ((double) res_angle[ 0 ] ) - 1 ) * 2 * pi - pi;
			for ( int l = 0 ; l < res_angle[ 1 ] ; l++ )
				{
				theta = ((double) l) / ( ((double) res_angle[ 1 ]) - 1 ) * pi;
				// Some gonimetric variables we'll constantly need, best to just calculate them once
				cost = cos( theta ); cosp = cos( phi ); sint = sin( theta ); sinp = sin( phi );
				sintcosp = sin( theta ) * cos( phi ); sintsinp = sin( theta ) * sin( phi );
				costsinp = cos( theta ) * sin( phi ); costcosp = cos( theta ) * cos( phi );

				// Reset
				L_phi = 0.0;
				L_theta = 0.0;
				N_phi = 0.0;
				N_theta = 0.0;
				for ( int dir = 0 ; dir < 3 ; dir++ )
					{
					if ( d == dir || d == NO_DIRECTION )
						{
						for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
							{
							for ( double x = 0.0 ; x < (double) size[ dir == 0 ? 1 : 0 ] ; x++ )
								{
								for ( double y = 0.0 ; y < (double) size[ dir == 2 ? 1 : 2 ] ; y++ )
									{
									C = polar( 1.0,
											( dir == 0 ? ( pos == 0 ? 1.0 : -1.0 ) * ( ( (double) size[ 0 ] ) - 1.0 ) / 2.0 : x - ( ( (double) size[ 0 ] ) - 1.0 ) / 2.0 ) * sintcosp / resolution +
											( dir == 1 ? ( pos == 0 ? 1.0 : -1.0 ) * ( ( (double) size[ 1 ] ) - 1.0 ) / 2.0 : ( dir == 0 ? x : y ) - ( ( (double) size[ 1 ] ) - 1.0 ) / 2.0 ) * sintsinp / resolution +
											( dir == 2 ? ( pos == 0 ? 1.0 : -1.0 ) * ( ( (double) size[ 2 ] ) - 1.0 ) / 2.0 : y - ( ( (double) size[ 2 ] ) - 1.0 ) / 2.0 ) * cost / resolution );

									// [ dir ][ pos ][ comp ][ x ][ y ]
									Mx = ( dir == 1 ? ( pos == 0 ? (complex<double>) _near_data[ 1 ][ pos ][ 1 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 1 ][ pos ][ 1 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 2 ? ( pos == 0 ? (complex<double>) _near_data[ 2 ][ pos ][ 1 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 2 ][ pos ][ 1 ][ (int)x ][ (int)y ] ) : 0.0 );
									My = ( dir == 2 ? ( pos == 0 ? (complex<double>) _near_data[ 2 ][ pos ][ 0 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 2 ][ pos ][ 0 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 0 ? ( pos == 0 ? (complex<double>) _near_data[ 0 ][ pos ][ 1 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 0 ][ pos ][ 1 ][ (int)x ][ (int)y ] ) : 0.0 );
									Mz = ( dir == 0 ? ( pos == 0 ? (complex<double>) _near_data[ 0 ][ pos ][ 0 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 0 ][ pos ][ 0 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 1 ? ( pos == 0 ? (complex<double>) _near_data[ 1 ][ pos ][ 0 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 1 ][ pos ][ 0 ][ (int)x ][ (int)y ] ) : 0.0 );

									Jx = ( dir == 1 ? ( pos == 0 ? (complex<double>) _near_data[ 1 ][ pos ][ 3 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 1 ][ pos ][ 3 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 2 ? ( pos == 0 ? (complex<double>) _near_data[ 2 ][ pos ][ 3 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 2 ][ pos ][ 3 ][ (int)x ][ (int)y ] ) : 0.0 );
									Jy = ( dir == 2 ? ( pos == 0 ? (complex<double>) _near_data[ 2 ][ pos ][ 2 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 2 ][ pos ][ 2 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 0 ? ( pos == 0 ? (complex<double>) _near_data[ 0 ][ pos ][ 3 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 0 ][ pos ][ 3 ][ (int)x ][ (int)y ] ) : 0.0 );
									Jz = ( dir == 0 ? ( pos == 0 ? (complex<double>) _near_data[ 0 ][ pos ][ 2 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 0 ][ pos ][ 2 ][ (int)x ][ (int)y ] ) : 0.0 ) - ( dir == 1 ? ( pos == 0 ? (complex<double>) _near_data[ 1 ][ pos ][ 2 ][ (int)x ][ (int)y ] : -1.0 * (complex<double>) _near_data[ 1 ][ pos ][ 2 ][ (int)x ][ (int)y ] ) : 0.0 );

									L_phi   += ( -1.0 * Mx * sinp + My * cosp ) * C;
									L_theta += ( Mx * costcosp + My * costsinp - Mz * sint ) * C;
									N_phi   += ( -1.0 * Jx * sinp + Jy * cosp ) * C;
									N_theta += ( Jx * costcosp + Jy * costsinp - Jz * sint ) * C;
									}
								}
							}
						}
					}
				norm = _size->x() * _size->y() * _size->z() * pow( resolution, 3 );

				_far_data_e_phi_mag[   k * res_angle[ 1 ] + l ] = (realnum) ( abs( L_theta - N_phi ) / norm );
				_far_data_e_phi_arg[   k * res_angle[ 1 ] + l ] = (realnum) arg( L_theta - N_phi );
				_far_data_e_theta_mag[ k * res_angle[ 1 ] + l ] = (realnum) ( abs( -1.0 * ( L_phi + N_theta ) ) / norm );
				_far_data_e_theta_arg[ k * res_angle[ 1 ] + l ] = (realnum) arg( -1.0 * ( L_phi + N_theta ) );

				_far_data_h_phi_mag[   k * res_angle[ 1 ] + l ] = (realnum) ( abs( -1.0 * ( N_theta + L_phi ) ) / norm );
				_far_data_h_phi_arg[   k * res_angle[ 1 ] + l ] = (realnum) arg( -1.0 * ( N_theta + L_phi ) );
				_far_data_h_theta_mag[ k * res_angle[ 1 ] + l ] = (realnum) ( abs( N_phi - L_theta ) / norm );
				_far_data_h_theta_arg[ k * res_angle[ 1 ] + l ] = (realnum) arg( N_phi - L_theta );
				}
			}

		for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
			{
			if ( d == dir_index || d == NO_DIRECTION )
				{
				for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
					{
					for ( int comp = 0 ; comp < 4 ; comp++ )
						{
						for ( int x = 0 ; x < size[ dir_index == 0 ? 1 : 0 ] ; x++ )
							{
							delete _near_data[ dir_index ][ pos ][ comp ][ x ];
							}
						delete _near_data[ dir_index ][ pos ][ comp ];
						}
					delete _near_data[ dir_index ][ pos ];
					}
				delete _near_data[ dir_index ];
				}
			}
		delete _near_data;
		_near_data = NULL;
		}
	all_wait();
}

void nf2ff:: allocate()
{
	if ( am_master() )
		{
		_near_data = new complex<realnum> ****[ 3 ];
		for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
			{
			if ( d == dir_index || d == NO_DIRECTION )
				{
				_near_data[ dir_index ] = new complex<realnum> ***[ 2 ];
				for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
					{
					_near_data[ dir_index ][ pos ] = new complex<realnum> **[ 4 ];
					for ( int comp = 0 ; comp < 4 ; comp++ )
						{
						_near_data[ dir_index ][ pos ][ comp ] = new complex<realnum> *[ size[ dir_index == 0 ? 1 : 0 ] ];
						for ( int x = 0 ; x < size[ dir_index == 0 ? 1 : 0 ] ; x++ )
							{
							_near_data[ dir_index ][ pos ][ comp ][ x ] = new complex<realnum> [ size[ dir_index == 2 ? 1 : 2 ] ];
							}
						}
					}
				}
			}
		}
	all_wait();
}

void nf2ff:: pass_data()
{
	complex<realnum> data_c_buf;
	complex<realnum> data_c;
	for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
		{
		if ( d == dir_index || d == NO_DIRECTION )
			{
			for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
				{
				for ( int comp = 0 ; comp < _snaps[ dir_index ][ pos ]->n_c ; comp++ )
					{
					for ( int n_0 = 0 ; n_0 < _snaps[ dir_index ][ pos ]->n_dims[ 0 ] ; n_0++ )
						{
						for ( int n_1 = 0 ; n_1 < _snaps[ dir_index ][ pos ]->n_dims[ 1 ] ; n_1++ )
							{
							data_c = 0.0;
							data_c_buf = 0.0;
							if ( _snaps[ dir_index ][ pos ]->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ 0 ] && _snaps[ dir_index ][ pos ]->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ 0 ]->N > 0 )
								{
								for ( int n = 0 ; n < _snaps[ dir_index ][ pos ]->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ 0 ]->N ; n++ )
									{
									data_c += _snaps[ dir_index ][ pos ]->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ 0 ]->dft[ n ];
									}
								data_c = data_c / (realnum) _snaps[ dir_index ][ pos ]->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ 0 ]->N;
								}
							for ( int proc = 0 ; proc < count_processors() ; proc++ )
								{
								data_c_buf = Ssend( proc, 0, data_c );
								if ( am_master() && ( data_c_buf.real() != 0.0 || data_c_buf.imag() != 0.0 ) )
									{
									data_c = data_c_buf;
									}
								}
							if ( am_master() )
								{
								_near_data[ dir_index ][ pos ][ comp ][ n_0 ][ n_1 ] = data_c;
								}
							}
						}
					}
				}
			}
		}
	all_wait();
}

void nf2ff:: output()
{
	if ( am_master() )
		{
		char * string = new char[ strlen( _name ) + 10 ];
		strcpy( string, _name );
		strcat( string, "-nf2ff.h5\0" );
		master_printf( "creating output file \"./%s\"...\n", string );

		_h5file = new h5file( string, h5file::WRITE, false );

		int dims[ 2 ] = { res_angle[ 0 ], res_angle[ 1 ] };

		_h5file->write( "ephi-mag", 2, &dims[ 0 ], _far_data_e_phi_mag, true );
		_h5file->write( "ephi-arg", 2, &dims[ 0 ], _far_data_e_phi_arg, true );
		_h5file->write( "etheta-mag", 2, &dims[ 0 ], _far_data_e_theta_mag, true );
		_h5file->write( "etheta-arg", 2, &dims[ 0 ], _far_data_e_theta_arg, true );
		_h5file->write( "hphi-mag", 2, &dims[ 0 ], _far_data_h_phi_mag, true );
		_h5file->write( "hphi-arg", 2, &dims[ 0 ], _far_data_h_phi_arg, true );
		_h5file->write( "htheta-mag", 2, &dims[ 0 ], _far_data_h_theta_mag, true );
		_h5file->write( "htheta-arg", 2, &dims[ 0 ], _far_data_h_theta_arg, true );

		delete string;
		delete _h5file;
		delete _far_data_e_phi_mag;
		delete _far_data_e_theta_mag;
		delete _far_data_e_phi_arg;
		delete _far_data_e_theta_arg;
		delete _far_data_h_phi_mag;
		delete _far_data_h_theta_mag;
		delete _far_data_h_phi_arg;
		delete _far_data_h_theta_arg;

		_h5file					= NULL;
		_far_data_e_phi_mag		= NULL;
		_far_data_e_theta_mag	= NULL;
		_far_data_e_phi_arg		= NULL;
		_far_data_e_theta_arg	= NULL;
		_far_data_h_phi_mag		= NULL;
		_far_data_h_theta_mag	= NULL;
		_far_data_h_phi_arg		= NULL;
		_far_data_h_theta_arg	= NULL;
		}
	all_wait();
}

void nf2ff:: create_snaps()
{
	char * string = new char[ strlen( _name ) + 32 ];
	_snaps = new snapshot **[ 3 ];
	for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
		{
		if ( d == dir_index || d == NO_DIRECTION )
			{
			_snaps[ dir_index ] = new snapshot *[ 2 ];
			for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
				{
				sprintf( string, "%s-%s%c-%f\0", _name, direction_name( (direction) dir_index ), pos == 0 ? 'p' : 'm', freq );
				_snaps[ dir_index ][ pos ] = new snapshot( _f,		// fields points
															4,		// number of components
															string, // name
															vec(	_center->x() + ( pos == 0 ? 1.0 : -1.0 ) * ( dir_index == 0 ? _size->x() / 2.0 : 0.0 ),		// center x
																	_center->y() + ( pos == 0 ? 1.0 : -1.0 ) * ( dir_index == 1 ? _size->y() / 2.0 : 0.0 ),		// center y
																	_center->z() + ( pos == 0 ? 1.0 : -1.0 ) * ( dir_index == 2 ? _size->z() / 2.0 : 0.0 ) ),	// center z
															vec(	dir_index == 0 ? 0.0 : _size->x(),		// size x
																	dir_index == 1 ? 0.0 : _size->y(),		// size y
																	dir_index == 2 ? 0.0 : _size->z() ),	// size z
															0, NO_DIRECTION,	// radius / direction, only for hemispherical snapshots
															freq, resolution );
				_snaps[ dir_index ][ pos ]->add_component( return_component( (direction) dir_index, 0 ), 0 );
				_snaps[ dir_index ][ pos ]->add_component( return_component( (direction) dir_index, 1 ), 1 );
				_snaps[ dir_index ][ pos ]->add_component( return_component( (direction) dir_index, 2 ), 2 );
				_snaps[ dir_index ][ pos ]->add_component( return_component( (direction) dir_index, 3 ), 3 );
				_snaps[ dir_index ][ pos ]->create();
				}
			}
		}
	delete string;
}

void nf2ff:: output_snaps()
{
	if ( am_master() )
		{
		char * string = new char[ strlen( _name ) + 32 ];
		int start[ 2 ] = { 0, 0 };
		int n_dims[ 2 ] = { 0, 0 };
		int rank = 2;

		realnum * _data_mag;
		realnum * _data_arg;

		for ( int dir_index = 0 ; dir_index < 3 ; dir_index++ )
			{
			if ( d == dir_index || d == NO_DIRECTION )
				{
				n_dims[ 0 ] = ( dir_index == 0 ? size[ 1 ] : size[ 0 ] );
				n_dims[ 1 ] = ( dir_index == 2 ? size[ 1 ] : size[ 2 ] );
				_data_mag = new realnum [ n_dims[ 0 ] * n_dims[ 1 ] ];
				_data_arg = new realnum [ n_dims[ 0 ] * n_dims[ 1 ] ];
				for ( int pos = 0 ; pos < ( d == NO_DIRECTION ? 2 : 1 ) ; pos++ )
					{
					sprintf( string, "%s-%s%c-%f.h5", _name, direction_name( (direction) dir_index ), pos == 0 ? 'p' : 'm', freq );
					master_printf( "creating output file \"./%s\"...\n", string );
					_h5file = new h5file( string, h5file::WRITE, false );
					for ( int comp = 0 ; comp < 4 ; comp++ )
						{
						if ( am_master() )
							{
							for ( int n_0 = 0 ; n_0 < n_dims[ 0 ] ; n_0++ )
								{
								for ( int n_1 = 0 ; n_1 < n_dims[ 1 ] ; n_1++ )
									{
									_data_mag[ n_0 * n_dims[ 1 ] + n_1 ] = abs( _near_data[ dir_index ][ pos ][ comp ][ n_0 ][ n_1 ] );
									_data_arg[ n_0 * n_dims[ 1 ] + n_1 ] = arg( _near_data[ dir_index ][ pos ][ comp ][ n_0 ][ n_1 ] );
									}
								}
							}
						sprintf( string, "%s-mag\0", component_name( return_component( (direction) dir_index, comp ) ) );
						_h5file->write( string, rank, &n_dims[ 0 ], _data_mag, true );
						sprintf( string, "%s-arg\0", component_name( return_component( (direction) dir_index, comp ) ) );
						_h5file->write( string, rank, &n_dims[ 0 ], _data_arg, true );
						}
					delete _h5file;
					}
				delete _data_mag;
				delete _data_arg;
				_h5file = NULL;
				_data_mag = NULL;
				_data_arg = NULL;
				}
			}
		delete string;
		}
	all_wait();
}

component nf2ff:: return_component( direction dir, int pos )
{
	switch( dir )
		{
		case X:
			switch( pos )
				{
				case 0: return Ey;
				case 1:	return Ez;
				case 2: return Hy;
				case 3:	return Hz;
				default: abort("bug - nf2ff return_component invalid pos");
				};
		case Y:
			switch( pos )
				{
				case 0: return Ex;
				case 1: return Ez;
				case 2: return Hx;
				case 3: return Hz;
				default: abort("bug - nf2ff return_component invalid pos");
				};
		case Z:
			switch( pos )
				{
				case 0: return Ex;
				case 1:	return Ey;
				case 2:	return Hx;
				case 3: return Hy;
				default: abort("bug - nf2ff return_component invalid pos");
				};
		default:
			 abort("bug - nf2ff return_component invalid direction");
		};
	return Ex; // This is never reached
}

mode_volume:: mode_volume( fields * f, const vec &center, const vec &size, double l, double n, double res, char * name, bool output )
{
	_center		= new vec( center.x(), center.y(), center.z() );
	_size		= new vec( size.x(), size.y(), size.z() );
	_name		= new char[ strlen( name ) + 1 ];
	strcpy( _name, name);

	_f			= f;
	freq		= l;
	refractive_index = n;
	resolution  = res;
	out			= output;

	_snap		= new snapshot( _f, 3, _name, vec( center.x(), center.y(), center.z() ), vec( size.x(), size.y(), size.z() ), 0, NO_DIRECTION, freq, resolution );
	_snap->add_component( Ex, 0 );
	_snap->add_component( Ey, 1 );
	_snap->add_component( Ez, 2 );
	_snap->create();

}

mode_volume:: ~mode_volume()
{
	delete _center;
	delete _size;
	delete _name;
	delete _snap;
}

void mode_volume:: local_calc()
{
	max_val = 0.0;
	vol = 0.0;

	double x_loc, y_loc, z_loc;
	int n_car[ 3 ];
	complex<double> temp[ 3 ];
	double local_val;

	n_car[ 0 ] = (int) ceil( _size->x() * resolution + 1.0 );
	n_car[ 1 ] = (int) ceil( _size->y() * resolution + 1.0 );
	n_car[ 2 ] = (int) ceil( _size->z() * resolution + 1.0 );

	for ( int n_0 = 0 ; n_0 < n_car[ 0 ] ; n_0++ )
		{
		for ( int n_1 = 0 ; n_1 < n_car[ 1 ] ; n_1++ )
			{
			for ( int n_2 = 0 ; n_2 < n_car[ 2 ] ; n_2++ )
				{
				for ( int comp = 0 ; comp < 3 ; comp++ )
					{
					temp[ comp ] = 0.0;
					if ( _snap->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ] )
						{
						for ( int n = 0 ; n < _snap->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->N ; n++ )
							{
							temp[ comp ] += (complex<double>)_snap->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->dft[ n ];
							}
						temp[ comp ] = temp[ comp ] / (double)_snap->_dft_chunk_array_ptr[ comp ][ n_0 ][ n_1 ][ n_2 ]->N;
						}
					}
				x_loc = _center->x() - _size->x() / 2.0 + ( n_car[ 0 ] != 1 || ( n_car[ 1 ] == 1 && n_car[ 2 ] == 1 ) ? ((double)n_0) : ( n_car[ 1 ] != 1 && n_car[ 2 ] != 1 ? ((double)n_2) : ((double)n_1) ) ) / resolution;
				y_loc = _center->y() - _size->y() / 2.0 + ( n_car[ 0 ] != 1 && ( n_car[ 1 ] != 1 || n_car[ 2 ] == 1 ) ? ((double)n_1) : ( n_car[ 0 ] == 1 && ( n_car[ 1 ] != 1 || n_car[ 2 ] == 1 ) ? ((double)n_0) : ((double)n_2) ) ) / resolution;
				z_loc = _center->z() - _size->z() / 2.0 + ( n_car[ 0 ] == 1 &&   n_car[ 1 ] == 1 && n_car[ 2 ] != 1   ? ((double)n_0) : ( n_car[ 2 ] != 1 && ( n_car[ 0 ] == 1 || n_car[ 1 ] == 1 ) ? ((double)n_1) : ((double)n_2) ) ) / resolution;

				temp[ 0 ] = temp[ 0 ] * conj( temp[ 0 ] ) + temp[ 1 ] * conj( temp[ 1 ] ) + temp[ 2 ] * conj( temp[ 2 ] );
				local_val = temp[ 0 ].real() * (double)_f->get_eps( vec( x_loc, y_loc, z_loc ) );
				//printf( "%f %f %f: %f\n", x_loc, y_loc, z_loc, _f->get_eps( vec( x_loc, y_loc, z_loc ) ) );
				if ( local_val > max_val )
					{
					max_val = local_val;
					}
				vol = vol + local_val;
				}
			}
		}
}

void mode_volume:: pass_data()
{
	double data_buf;
	double data_buf_max;
	double vol_tot_buf = 0.0;
	for ( int proc = 0 ; proc < count_processors() ; proc++ )
		{
		data_buf = vol;
		data_buf_max = max_val;
		send( proc, 0, &data_buf );
		send( proc, 0, &data_buf_max );
		if ( am_master() )
			{
			vol_tot_buf += data_buf;
			if ( data_buf_max > max_val )
				{
				max_val = data_buf_max;
				}
			}
		}
	vol = vol_tot_buf / ( max_val * pow( ( 1 / freq ) / refractive_index, 3 ) * pow( resolution, 3 ) );
}

void mode_volume:: output()
{
	_f->am_now_working_on( ModeVolCalc );
	local_calc();
	pass_data();
	_f->finished_working();
	master_printf( "mode volume '%s' = %f [(wavelength/n)%c]\n", _name, vol, ((char)179) );
	all_wait();
	if ( out )
		{
		_snap->output();
		}
}


/***************************************************************************/

} // namespace meep
