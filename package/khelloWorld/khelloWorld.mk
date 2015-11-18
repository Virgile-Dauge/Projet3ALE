KHELLOWORLD_VERSION = 1.0
KHELLOWORLD_SITE = $(TOPDIR)/package/khelloWorld/driver
KHELLOWORLD_SITE_METHOD = local
KHELLOWORLD_LICENSE = GPLv3+

KHELLOWORLD_DEPENDENCIES = linux

define KHELLOWORLD_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define KHELLOWORLD_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef



$(eval $(generic-package))
