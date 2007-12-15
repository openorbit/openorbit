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
	
	
# Force generation of header-file
cdef public int dummy

