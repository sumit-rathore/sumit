import System.Windows.Forms as Forms
import Scripts.ipxact_xml as ipx
import icron_lib.icron_file_parser
from cobs_logger import cbs_logger


class IPXTree(object):
    """Convenience class that provides a .NET Forms.TreeView of an IP-XACT
    memory hierarchy. Clients are responsible for assigning event handlers
    to make use of the TreeView.
    """

    def __init__(self, dev, parent):
        self.treeView = Forms.TreeView()
        self.registerDataModel = self._configureDataModel(dev)
        self._populateTreeView()
        self.parent = parent
        self.rClickedNode = None
        def expAll(sender, args):
            self.treeView.ExpandAll()

        def collAll(sender, args):
            self.treeView.CollapseAll()

        def expNode(sender, args):
            self.treeView.SelectedNode.Expand()

        def collNode(sender, args):
            self.treeView.SelectedNode.Collapse()

        def expNodeAll(sender, args):
            self.treeView.SelectedNode.ExpandAll()

        def collNodeAll(sender, args):
            self.treeView.SelectedNode.Collapse(False)

        def clearDataGridView(sender, args):
            if hasattr(self.parent, 'clearDataGridView'):
                self.parent.clearDataGridView()

        def setRightClickedNode(sender, args):
            if args.Button == Forms.MouseButtons.Right:
                self.rClickedNode = args.Node

        def setKybdMenuNode(sender, args):
            if args.KeyCode == Forms.Keys.Apps :
                self.rClickedNode = self.parent.ipxTree.treeView.SelectedNode

        def addNodeAsPersistentDGVRow(sender, args):
            if hasattr(self.parent, 'treeViewNodeAddPersistentHandler'):
                self.parent.treeViewNodeAddPersistentHandler(self.rClickedNode)

        def filterTree(sender, args):
            # reset our stored called node
            if hasattr(self.parent, 'filterNode'):
                # collapse all under currently highlighed node
                self.rClickedNode.Collapse()
                self.parent.srchFound = False
                self.parent.srchLevelFound = 0
                self.parent.filterNode = None
                self.parent.filterTree(self.rClickedNode, self.parent.searchTextBox.Text)

        self.treeView.NodeMouseClick += setRightClickedNode
        self.treeView.KeyUp += setKybdMenuNode
        self.treeView.ContextMenu = Forms.ContextMenu()
        menuItems = self.treeView.ContextMenu.MenuItems
        menuItems.Add(Forms.MenuItem("Expand all", expAll))
        menuItems.Add(Forms.MenuItem("Collapse all", collAll))
        menuItems.Add(Forms.MenuItem("Expand Node", expNode))
        menuItems.Add(Forms.MenuItem("Collapse Node", collNode))
        menuItems.Add(Forms.MenuItem("Expand Node Full Subtree", expNodeAll))
        menuItems.Add(Forms.MenuItem("Collapse Node Full Subtree", collNodeAll))
        if hasattr(self.parent, "com"):
            menuItems.Add(Forms.MenuItem("Clear Register Data", clearDataGridView))
            menuItems.Add(Forms.MenuItem("Monitor", addNodeAsPersistentDGVRow))
            menuItems.Add(Forms.MenuItem("Filter", filterTree))


    def _configureDataModel(self, dev):
        try:
            projectName = dev.device_name
            iregister_settings = dev.iregister_model.register_settings
            spiritBaseUri = 'http://www.spiritconsortium.org/XMLSchema/SPIRIT/'
            if "blackbird" == projectName:
                namespaces = {'spirit': spiritBaseUri + '1685-2009'}
            elif "goldenears" == projectName:
                namespaces = {'spirit': spiritBaseUri + '1.4', 'pdt': 'PDT'}
            else:
                raise Exception('Unknown project name: %s' % projectName)

            registerDataModel = ipx.RegisterDataModel(namespaces)
            for (base_address, xml_str) in iregister_settings:
                registerDataModel.add_component_from_string(xml_str, base_address)
            return registerDataModel
        except:
            cbs_logger.exception("Got an error when configure data model")

    def _populateTreeView(self):
        # In Python 2, a dict can function as a container for closed-over
        # variables. d['path'] represents the path taken through the tree to
        # reach the current node.
        d = {'path': []}
        def addNode(node, depth):
            if hasattr(node, 'name'):
                n = IPXTreeNode(node.name)
                n.ipxNode = node
                if depth == 0:
                    self.treeView.Nodes.Add(n)
                elif depth >= len(d['path']):
                    d['path'][-1].Nodes.Add(n)
                else:  # depth < len(d['path'])
                    d['path'] = d['path'][:depth]
                    d['path'][-1].Nodes.Add(n)
                d['path'].append(n)
        self.treeView.BeginUpdate()
        self.registerDataModel.traverse(addNode)
        self.treeView.EndUpdate()


class IPXTreeNode(Forms.TreeNode):
    """Python class wrapper for .NET's Forms.TreeNode. This is nececessary
    in order to assign new attributes to a .NET class.
    """
    pass
