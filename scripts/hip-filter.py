#! /usr/bin/env python

#   Copyright 2006 Mattias Holm <mattias.holm(at)openorbit.org>

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


# Takes the raw hiparcos database (assumed to be unzipped), filters out the
# interesting data (with a threshold for magnitudes) and saves the
# textual representation of the filtered data to a simple CSV-file.
from string import *
import sys

inFileName = sys.argv[1]
outFileName = sys.argv[2]
vmagLimit = float(sys.argv[3])

# List of keys used to access the fields in the lines
hipKeys = [
	"Catalog", "HIP", "Proxy", "RAhms","DEdms", "Vmag","VarFlag", "r_Vmag",
	"RAdeg", "DEdeg", "AstroRef", "Plx", "pmRA", "pmDE", "e_RAdeg", "e_DEdeg",
	"e_Plx", "e_pmRA", "e_pmDE", "DE:RA", "Plx:RA", "Plx:DE", "pmRA:RA",
	"pmRA:DE", "pmRA:Plx", "pmDE:RA", "pmDE:DE", "pmDE:Plx", "pmDE:pmRA", "F1",
	"F2", "---", "BTmag", "e_BTmag", "VTmag", "e_VTmag", "m_BTmag", "B-V",
	"e_B-V", "r_B-V", "V-I", "e_V-I", "r_V-I", "CombMag", "Hpmag", "e_Hpmag",
	"Hpscat", "o_Hpmag", "m_Hpmag", "Hpmax", "HPmin", "Period", "HvarType",
	"moreVar", "morePhoto", "CCDM", "n_CCDM", "Nsys", "Ncomp", "MultFlag",
	"Source", "Qual", "m_HIP", "theta", "rho", "e_rho", "dHp", "e_dHp",
	"Survey", "Chart", "Notes", "HD", "BD", "CoD", "CPD", "(V-I)red", "SpType",
	"r_SpType"
]

# Assign numbers to the elements
hipDict = {}

for key, val in zip(hipKeys, range(0, len(hipKeys))):
	hipDict[key] = val


# The data that is to be extracted
hipGetKeys = [
	"Vmag", "RAdeg", "DEdeg", "BTmag", "VTmag", "B-V", "V-I"
]



def filterLine(line, entries, vmagLimit):
    lst = line.split('|')
    for i in range(0, len(lst)):
        lst[i] = lst[i].strip()
    result = []
	
    if len(lst) != len(hipKeys):
        return None
	
    if lst[hipDict["Vmag"]] == "":
        return None
	
    if float(lst[hipDict["Vmag"]]) > vmagLimit:
        return None

    for entry in entries:
        keyId = hipDict[entry]
        if lst[keyId] == "":
            return None
        result.append(lst[keyId])
    
    return result

f = open(inFileName)
o = open(outFileName, "w")
try:
    for line in f:
        lst = filterLine(line, hipGetKeys, vmagLimit)
        if lst != None:
            o.write(",".join(lst))
            o.write("\n")
finally:
    f.close()
    o.close()
