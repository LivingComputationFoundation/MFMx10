import sys
import traceback

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

def die(message):
    eprint(f"Error: {message}")
    s = traceback.extract_stack()
    eprint(' '.join(traceback.format_list(s[:-1])))
    exit(2)

import dumper
dumper.max_depth = 10
#dumper.instance_dump = 'all'



import contextlib

@contextlib.contextmanager
def objAttrChange(obj, attr_name, new_value):
    """
    A context manager to temporarily change the value of an attribute
    on an object, and ensure it's restored afterwards.

    Args:
        obj: The object whose attribute you want to change.
        attr_name: The name of the attribute (as a string).
        new_value: The temporary value to set the attribute to.
    """
    # Store the original value
    original_value = getattr(obj, attr_name)

    # Set the new value
    setattr(obj, attr_name, new_value)

    try:
        # Yield control to the 'with' block
        yield
    finally:
        # This block is guaranteed to execute, even if an exception occurs
        # Restore the original value
        setattr(obj, attr_name, original_value)

def dumpAlls(obj):
    with objAttrChange(dumper,'instance_dump','all'):
        return dumper.dumps(obj)
    
