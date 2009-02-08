#   The contents of this file are subject to the Mozilla Public License
#   Version 1.1 (the "License"); you may not use this file except in compliance
#   with the License. You may obtain a copy of the License at
#   http://www.mozilla.org/MPL/
#   
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#   
#   The Original Code is the Open Orbit space flight simulator.
#   
#   The Initial Developer of the Original Code is Mattias Holm. Portions
#   created by the Initial Developer are Copyright (C) 2006 the
#   Initial Developer. All Rights Reserved.
#   
#   Contributor(s):
#       Mattias Holm <mattias.holm(at)contra.nu>.
#   
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 2 or later (the "GPL"), or
#   the GNU Lesser General Public License Version 2.1 or later (the "LGPL"), in
#   which case the provisions of GPL or the LGPL License are applicable instead
#   of those above. If you wish to allow use of your version of this file only
#   under the terms of the GPL or the LGPL and not to allow others to use your
#   version of this file under the MPL, indicate your decision by deleting the
#   provisions above and replace  them with the notice and other provisions
#   required by the GPL or the LGPL.  If you do not delete the provisions
#   above, a recipient may use your version of this file under either the MPL,
#   the GPL or the LGPL."

import io       # I/O module, allowing the binding of key handlers
import config   # config, allows one to set config values
import orbits
import res, sg, sim, texture

import yaml

import math
import ConfigParser
import sys
import os

from os import environ

print "Running post init script..."

def loadStars():
    f = open(res.getPath("data/stars.csv"))
    skyDrawable = sg.SkyDrawable()
    for line in f:
        vmag, ra, dec, btmag, vtmag, b_v, v_i = tuple(line.split(","))
        skyDrawable.addStar(math.radians(float(ra)), math.radians(float(dec)),
                            float(vmag), float(b_v))
    
    
    f.close()
    return skyDrawable
    
class UnitParseError(Exception):
    def __init__(self, descr):
        self.descr = descr
    def __str__(self):
        return repr(self.descr)

class UnitRangeError(Exception):
    def __init__(self, descr):
        self.descr = descr
    def __str__(self):
        return repr(self.descr)


def parseMass(str):
    tokens = str.split()
    if len(tokens) != 2:
        raise UnitParseError("Token count wrong")

    if tokens[1] == "kg":
        return float(tokens[0])
    elif tokens[1] == "Zg":
        return float(tokens[0]) * (10.0**21.0 / 10.0**3.0)
    elif tokens[1] == "Yg":
        return float(tokens[0]) * (10.0**24.0 / 10.0**3.0)
    else:
        raise UnitParseError("Unknown unit in mass context: " + tokens[1])

def parseAngle(str):
    tokens = str.split()
    if len(tokens) != 2:
        raise UnitParseError("Token count wrong")

    if tokens[1] == "rad":
        return float(tokens[0])
    elif tokens[1] == "deg":
        return radians(float(tokens[0]))
    else:
        raise UnitParseError("Unknown unit in angle context: " + tokens[1])

def parseDistance(str):
    tokens = str.split()
    if len(tokens) != 2:
        raise UnitParseError("Token count wrong")

    if tokens[1] == "m":
        return float(tokens[0])
    elif tokens[1] == "km":
        return float(tokens[0])
    elif tokens[1] == "au":
        return float(tokens[0])
    elif tokens[1] == "ly":
        return float(tokens[0])
    elif tokens[1] == "pc":
        return float(tokens[0])    
    else:
        raise UnitParseError("Unknown unit in distance context: " + tokens[1])

def parsePreasure(str):
    tokens = str.split()
    if len(tokens) != 2:
        raise UnitParseError("Token count wrong")

    if tokens[1] == "kPa":
        return float(tokens[0]) * 1000.0
    elif tokens[1] == "Pa":
        return float(tokens[0])
    else:
        raise UnitParseError("Unknown unit in preasure context: " + tokens[1])


def parseTemp(str):
    tokens = str.split()
    if len(tokens) != 2:
        raise UnitParseError("Token count wrong")

    if tokens[1] == "K":
        val = float(tokens[0])
        if val < 0.0:
            raise UnitRangeError("Temperature may not be less than 0 K")
        return val
    elif tokens[1] == "C":
        val = float(tokens[0]) + 273.15
        if val < 0.0:
            raise UnitRangeError("Temperature may not be less than -273.15 C")
        return val
    else:
        raise UnitParseError("Unknown unit in temp context: " + tokens[1])

def parseTime(str):
    tokens = str.split()
    if len(tokens) == 2:
        if tokens[1] == "days":
            return float(tokens[0]) * 3600.0 * 24.0
        elif tokens[1] == "s":
            return float(tokens[0])
        elif tokens[1] == "min":
            return float(tokens[0]) * 60.0
        else:
            raise UnitParseError("Unknown unit in time context: " + tokens[1])
    elif len(tokens) == 6:
        if tokens[1] == "h" and tokens[3] == "min" and tokens[5] == "s":
            return float(tokens[0]) * 3600.0 + float(tokens[2]) * 60.0\
                + float(tokens[4])
        else:
            raise UnitParseError("Unknown unit in time context: "
                                 + tokens[1] + " "
                                 + tokens[3] + " "
                                 + tokens[5])
    elif len(tokens) == 4:
        if tokens[1] == "h" and tokens[3] == "min":
             return float(tokens[0]) * 3600.0 + float(tokens[2]) * 60.0
        else:
            raise UnitParseError("Unknown unit in time context: "
                  + tokens[1] + " "
                  + tokens[3])
    else:
        raise UnitParseError("Token count wrong in %s" % (str))


def SemiMin(semiMaj, ecc):
  return math.sqrt(1.0 - ecc**2.0) * semiMaj
  
def addSgObj(scene, renderOpts):
  if renderOpts["model"] == "sphere":
    pass
  elif renderOpts["model"] == "mesh":
    pass
  #texture.load(renderOpts["texture"], renderOpts["texture"])
  
def addStar(scene, parent, name, body):
    try:
        orbit = body["orbit"]
        mass = parseMass(body["mass"])
        radius = parseDistance(body["radius"])
        axialPeriod = parseTime(orbit["axial-period"])
        rendOpts = body["rendering"]
    except KeyError, err:
        print "error: missing key in star %s (%s)" % (name, err.args)
        sys.exit(1)
    star = orbits.OrbitSys(name, mass, 0.0, 0.0)
    if parent:
        parent.addChild(star)
    #star.addObj(name, mradius, 0.0, mass)

    if body.has_key("satellites"):
        for key in body["satellites"].keys():
            newScene = sg.Scene().init(key)
            scene.addChild(newScene)
            addBody(newScene, star, key, body["satellites"][key])

    # not pretty, but works for now (i.e. will bomb with more than one star)
    sim.setOrbSys(star)
    
    #scene.connectToOdeObj(star.getBody())
    scene.connectToOrbSys(star)
    
    addSgObj(scene, rendOpts)
    
def addPlanet(scene, parent, name, body):
    try:
        orbit = body["orbit"]
        mass = parseMass(body["mass"])
        radius = parseDistance(body["radius"])
        axialPeriod = parseTime(orbit["axial-period"])
        rendOpts = body["rendering"]
        semiMaj = parseDistance(orbit["semi-major-axis"])
        ecc = orbit["eccentricity"]
        semiMin = SemiMin(semiMaj, ecc)
    except KeyError, err:
        print "error: missing key in planet %s (%s)" % (name, err.args)
        print "       since you missed this key, the planet will not be added"
        return
        
    planet = orbits.OrbitSys(name, mass, semiMaj, semiMin)
    if parent:
        parent.addChild(planet)
    #planet.addObj(name, radius, 0.0, mass)
    
    if body.has_key("satellites"):
        for key in body["satellites"].keys():
            addBody(scene, planet, key, body["satellites"][key])

    scene.connectToOrbSys(planet)
    #scene.connectToOdeObj(planet.getBody())
    addSgObj(scene, rendOpts)
    
def addMoon(scene, parent, name, body):
    try:
        orbit = body["orbit"]
        mass = parseMass(body["mass"])
        radius = parseDistance(body["radius"])
        axialPeriod = parseTime(orbit["axial-period"])
        rendOpts = body["rendering"]
        semiMaj = parseDistance(orbit["semi-major-axis"])
        ecc = orbit["eccentricity"]
        semiMin = SemiMin(semiMaj, ecc)
    except KeyError, err:
        print "error: missing key in moon %s (%s)" % (name, err.args)
        print "       the moon will not be added without these parameters being set"
        return
    moon = orbits.OrbitSys(name, mass, semiMaj, semiMin)
    if parent:
        parent.addChild(moon)
    #parent.addObj(name, radius, 0.0, 0.0)
    #scene.connectToOrbSys(moon)

    addSgObj(scene, rendOpts)
    
def addBody(scene, parent, name, body):
    """docstring for addBody"""
    # get known attributes of a body
    try:
        if body["kind"] == "star":
            addStar(scene, parent, name, body)
        elif body["kind"] == "planet":
            addPlanet(scene, parent, name, body)
        elif body["kind"] == "moon":
            addMoon(scene, parent, name, body)
    except KeyError:
        print "key error in body (no key 'kind' in %s)" % (name)
        sys.exit(1)

def loadSolYaml(sgr):
    """Loads the yaml description of the solar system"""
    f = file(res.getPath("data/solsystem.yaml"))
    solsys = yaml.load(f)
    f.close()
    for key in solsys.keys():
        if key != "epoch":
            addBody(sgr.getRoot(), None, key, solsys[key])
    
def _test():
    val = parseTime("1.0 days")
    assert(val == 3600.0*24.0)
    val = parseTime("2.0 h 20 min 30 s")
    assert(val == 2.0*3600.0 + 20.0*60.0 + 30.0)
    val = parseDistance("20 m")
    assert(val == 20.0)
    val = parsePreasure("100.0 kPa")
    assert(val == 100.0 * 1000.0)
    val = parsePreasure("100.0 Pa")
    assert(val == 100.0)
    

    
#if __name__ == "__main__":
#    _test()
#else:
stars = loadStars()

sgr = sg.Scenegraph().new()
sgr.setBackground(stars)
cam = sg.FreeCam()
cam.setParams(sgr, sgr.getRoot(), 0.0, 0.0, 0.0, 0.0, 0.0, 0.0)
sgr.setCam(cam)

sim.setSg(sgr)


loadSolYaml(sgr)
