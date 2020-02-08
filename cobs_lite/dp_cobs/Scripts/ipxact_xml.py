from abc import ABCMeta, abstractmethod
import itertools as it
import os
import sys
import xml.etree.ElementTree as ET


class Expandable:
    """Abstract mixin for a class representing an IP-XACT memory element
    type which may correspond to >=1 physical memory elements of the same type.
    """
    __metaclass__ = ABCMeta

    @abstractmethod
    def expand(self):
        """Returns an iterable of instances of the inheriting class
        (including self), recursively expanded w.r.t. the dimensioned
        memory element's size and index.
        """
        pass


class AddressableDescendants(object):
    """Mixin for a class representing an IP-XACT element with addressable
    descendants.
    """
    def collect_descendants(self, elem, ns, base_address, paths2ctors, expand):
        chained_it = None
        for path, ctor in paths2ctors.items():
            # TODO: inheritors of this class should have their __init__ methods
            # changed to take all but their first two args (self, elem) as
            # **kwargs. This will allow them to accept different
            # parameter lists.
            obj_it = it.imap(ctor, elem.iterfind(path, ns),
                             it.repeat(ns), it.repeat(base_address),
                             it.repeat(0), it.repeat(expand))
            if expand:
                obj_it = it.imap(ctor.expand, obj_it)
                # Flatten
                obj_it = (y for x in obj_it for y in x)

            chained_it = it.chain(chained_it, obj_it) if chained_it else obj_it
        l = list(chained_it)
        l.sort(key=lambda r: r.address)
        return l


def build_description(parent_elem, namespaces):
    """Build up a description string, accounting for the differences in PDT-
    and IDS-generated XML.
    """
    try:
        brief_path = 'spirit:vendorExtensions/pdt:briefDescription'
        brief = parent_elem.find(brief_path, namespaces).text
        if brief is None:
            brief = ''
        brief += os.linesep
    except SyntaxError:
        # pdt not in ns
        brief = ''
    try:
        description = parent_elem.find('spirit:description', namespaces).text
        if description is None:
            description = ''
    except AttributeError:
        description = ''
    return brief + description


def expand_prefix(str_to_expand, namespaces):
    """Return the expanded form of a prefixed XML tag. E.g.,
    expand_prefix('prefix:sometag', {'prefix', 'full_uri'}) would return
    '{full_uri}sometag'.
    """
    if ':' in str_to_expand:
        prefix, tag = str_to_expand.split(':', 1)
        return "{%s}%s" % (namespaces[prefix], tag)
    return str_to_expand


def print_node(node, depth=0):
    text = None
    if hasattr(node, 'name'):
        text = '  ' * depth + node.name
    if hasattr(node, 'address'):
        text += ': 0x%08x' % node.address
    if text:
        print text


class RegisterDataModel(object):
    """A class representing the root of an IP-XACT memory element hierarchy.
    Note that this class is not defined by IP-XACT.
    """
    def __init__(self, namespaces):
        # Note: the namespaces dictionary must contain a 'spirit' key
        self.children = []
        self.ns = namespaces
        self.name = 'RegisterDataModel'

    def add_component_from_file(self, xml_file, base_address=0):
        tree = ET.parse(xml_file)
        self._add_component(tree.getroot(), base_address)

    def add_component_from_string(self, xml_string, base_address=0):
        root = ET.fromstring(xml_string)
        self._add_component(root, base_address)

    def traverse(self, f):
        """Performs a depth-first traversal on the tree rooted at self,
        calling f(node, depth) at each node.
        """
        def _traverse(obj, f, depth=0):
            f(obj, depth)
            try:
                children = obj.children
            except AttributeError:
                return  # we've hit a leaf node
            for c in children:
                _traverse(c, f, depth+1)
        _traverse(self, f)

    def print_tree(self):
        self.traverse(f=print_node)

    def _add_component(self, root, base_address=0):
        # Workaround for Python issue 17011, which is not yet fixed in
        # our version of IronPython (2.7.5). This bug is fixed in
        # Python 2.7.6+ and 3.3.3+, so we can remove the workaround
        # when we upgrade.
        import xml.etree.ElementPath as EP
        EP._cache.clear()

        if root.tag != expand_prefix('spirit:component', self.ns):
            raise Exception('Doc is not rooted at a component element')
        self.children.append(Component(root, self.ns, base_address))


class Component(object):
    def __init__(self, component_elem, namespaces, base_address=0):
        self.ns = namespaces
        self.name = component_elem.find('spirit:name', self.ns).text
        # Note: we omit memoryMaps elements from the hierarchy because
        # they don't seem to add much useful information.
        mm_path = 'spirit:memoryMaps/spirit:memoryMap'
        mm_iter = component_elem.iterfind(mm_path, self.ns)
        self.children = [MemoryMap(mm, self.ns, base_address) for mm in mm_iter]


class MemoryMap(object):
    def __init__(self, mm_elem, namespaces, base_address=0):
        self.ns = namespaces
        self.name = mm_elem.find('spirit:name', self.ns).text
        # Currently we are only interested in addressBlock children
        ab_path = 'spirit:addressBlock'
        ab_iter = mm_elem.iterfind(ab_path, self.ns)
        self.children = [
                ab for uab in ab_iter
                for ab in AddressBlock(uab, self.ns, base_address, 0,
                                       True).expand()]


class AddressBlock(AddressableDescendants, Expandable, object):
    def __init__(self, elem, namespaces, external_base_address=0, index=0,
                 expand_descendants=False):
        self.ns = namespaces
        self.elem = elem
        self.index = index
        self.name = self.elem.find('spirit:name', self.ns).text
        self.range = int(self.elem.find('spirit:range', self.ns).text, 0)

        self.external_base_address = external_base_address
        base_address = int(self.elem.find('spirit:baseAddress', self.ns).text, 0)
        self.address = external_base_address + base_address + self.index * self.range

        try:
            dim_path = 'spirit:vendorExtensions/ids_properties/repeat'
            self.dim = int(self.elem.find(dim_path, self.ns).text, 0)
            if self.dim > 1:
                self.name += "[%d]" % self.index
        except AttributeError:
            self.dim = 1

        self.children = self.collect_descendants(
                self.elem, self.ns, self.address,
                {'spirit:register': Register,
                 'spirit:registerFile': RegisterFile},
                expand_descendants)

    def expand(self):
        return ([self] + [AddressBlock(self.elem, self.ns,
                                       self.external_base_address,
                                       i, True) for i in xrange(1, self.dim)])


class RegisterFile(AddressableDescendants, Expandable, object):
    def __init__(self, elem, namespaces, base_address,
                 index=0, expand_descendants=False):
        self.ns = namespaces
        self.elem = elem
        self.index = index
        self.name = self.elem.find('spirit:name', self.ns).text
        self.range = int(self.elem.find('spirit:range', self.ns).text, 0)
        offset = int(self.elem.find('spirit:addressOffset', self.ns).text, 0)
        self.base_address = base_address
        self.address = base_address + offset + self.index * self.range

        try:
            self.dim = int(self.elem.find('spirit:dim', self.ns).text, 0)
            if self.dim > 1:
                self.name += "[%d]" % self.index
        except AttributeError:
            self.dim = 1

        self.children = self.collect_descendants(
                self.elem, self.ns, self.address,
                {'spirit:register': Register,
                 'spirit:registerFile': RegisterFile},
                expand_descendants)

    def expand(self):
        return ([self] +
                [RegisterFile(self.elem, self.ns, self.base_address, i, True)
                for i in xrange(1, self.dim)])


class Register(Expandable, object):
    def __init__(self, elem, namespaces, base_address, index=0, _unused=0):
        self.ns = namespaces
        self.elem = elem
        self.index = index
        self.name = self.elem.find('spirit:name', self.ns).text
        # TODO setter for bit_width that checks bit_width % 8 == 0
        self.bit_width = int(self.elem.find('spirit:size', self.ns).text, 0)
        offset = int(self.elem.find('spirit:addressOffset', self.ns).text, 0)
        self.base_address = base_address
        self.address = base_address + offset + self.index * (self.bit_width/8)
        self.description = build_description(self.elem, self.ns)
        reset_value = int(
                self.elem.find('spirit:reset/spirit:value', self.ns).text, 0)
        reset_mask = int(
                self.elem.find('spirit:reset/spirit:mask', self.ns).text, 0)
        self.masked_reset_value = reset_value & reset_mask
        self.bitfields = [BitField(bf, self.ns, self.masked_reset_value) for
                          bf in self.elem.iterfind('spirit:field', self.ns)]

        try:
            self.dim = int(self.elem.find('spirit:dim', self.ns).text, 0)
            if self.dim > 1:
                self.name += "[%d]" % self.index
        except AttributeError:
            self.dim = 1

    def expand(self):
        return ([self] + [Register(self.elem, self.ns, self.base_address, i)
                          for i in xrange(1, self.dim)])


class BitField(object):
    def __init__(self, elem, namespaces, masked_parent_reset_value):
        self.ns = namespaces
        self.elem = elem
        self.name = self.elem.find('spirit:name', self.ns).text
        self.bit_width = int(
                self.elem.find('spirit:bitWidth', self.ns).text, 0)
        if self.bit_width <= 0:
            raise ValueError('BitField width cannot be <= 0')
        self.bit_offset = int(
                self.elem.find('spirit:bitOffset', self.ns).text, 0)
        self.reset_value = hex(int(
                ((1 << self.bit_width) - 1)
                & (masked_parent_reset_value >> self.bit_offset)))
        self.access = self.elem.find('spirit:access', self.ns).text
        self.description = build_description(self.elem, self.ns)
        try:
            self.value_type = self.elem.find(
                'spirit:vendorExtensions/pdt:valueType', self.ns).text
        except (SyntaxError, AttributeError):
            self.value_type = 'hexadecimal'

        # Dynamic attributes
        self.current_value = None


# In-file tests because the Cobbes component is structured in a way that is
# not conducive to separarate-file tests. Test with:
# $ cd /path/to/cobs_component/Scripts
# $ python -m unittest ipxact_xml
import unittest
class TestRegisterDataModel(unittest.TestCase):
    def test_register_data_model_ids(self):
        # Note the hardcoded path. This only works on icronsdev1
        # and may need to be adjusted in the future.
        reg_path= ('/data/engdev/designs/blackbird/released/dd/'
                   'BLACKBIRD_20160127T133212/src/f_blackbird_kc705/'
                   'regs/ids/bb_chip/bb_chip_regs.ipxact.xml')
        ns = {'spirit':
              'http://www.spiritconsortium.org/XMLSchema/SPIRIT/1685-2009'}
        rdm = RegisterDataModel(ns)
        rdm.add_component_from_file(reg_path)
        rdm.print_tree()

    def test_register_data_model_pdt(self):
        # Note the hardcoded paths. These only work on icronsdev1
        # and may need to be adjusted in the future.
        reg_path_base = ('/data/engdev/designs/goldenears/released/asic/'
                         'build_2014_09_08_v01_02_00_eco1/rls/docs/')
        reg_paths = [
            reg_path_base + 'clm_comp.xml',
            reg_path_base + 'grg_comp.xml',
            reg_path_base + 'ulmii_comp.xml',
            reg_path_base + 'xcsr_comp.xml',
            reg_path_base + 'xlr_comp.xml',
            reg_path_base + 'xrr_comp.xml',
        ]
        ns = {
            'spirit': 'http://www.spiritconsortium.org/XMLSchema/SPIRIT/1.4',
            'pdt': 'PDT',
        }
        rdm = RegisterDataModel(ns)
        for p in reg_paths:
            rdm.add_component_from_file(p)
        rdm.print_tree()

    def runTest(self):
        self.test_register_data_model_ids()
        self.test_register_data_model_pdt()

