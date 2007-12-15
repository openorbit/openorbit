#! /usr/bin/env python

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
