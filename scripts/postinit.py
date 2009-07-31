#   Copyright 2006,2009 Mattias Holm <mattias.holm(at)openorbit.org>

#   This file is part of Open Orbit.
#
#   Open Orbit is free software: you can redistribute it and/or modify
#   it under the terms of the GNU Lesser General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   Open Orbit is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with Open Orbit.  If not, see <http://www.gnu.org/licenses/>.


import ooscript

print "Running post init script..."

def SemiMin(semiMaj, ecc):
  return math.sqrt(1.0 - ecc**2.0) * semiMaj


stars = ooscript.SkyDrawable("data/stars.csv")
sgr = ooscript.Scenegraph().new()
sgr.setBackground(stars)

ooscript.setSg(sgr)
osys = ooscript.OrbitSys().new(sgr, "data/solsystem.hrml")
ooscript.setOrbSys(osys)

cam = ooscript.FreeCam()
cam.setParams(sgr, sgr.getScene("Sol/Jupiter/Io"),
              #6.96e8 + 1500000.0,
              1821.6e3 + 100000.0,
              #6.37e6 + 15000000.0,
              #1.74e6 + 100000.0,
              #- (1.74e6 + 100000.0),
              0.0,
#              + (1.74e6 + 100000.0),
              0.0,
              0.0, 1.0, 0.0)
sgr.setCam(cam)
