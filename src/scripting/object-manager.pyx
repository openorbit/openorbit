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

cdef extern from "object-manager.h":
    om_object_t* om_new_object(void *ctxt, char *class_name, char *object_name)
    void om_delete_object(void *obj)
	const char*	om_get_object_name(void *obj)
	void* om_get_object_data(void *obj)
	void* om_get_object_from_ptr(void *ctxt, void *address)
	bool om_object_is_class(void *obj, const char *class_name)
	bool om_conforms_to_iface(void *obj, const char *iface)
	void* om_get_concrete_obj(void *ctxt, const char *object_name)
	void* om_get_concrete_prop(void *obj, const char *prop_name)
	
	
# Generci class for object manager object

class OMObject(object):
    def __init__(self, class, name):
        pass
    def __getattribute__(self, key):
        pass
    def __setattr__(self, key, val):
        pass
