PINCONTROLLER_VERSION = 1.0
PINCONTROLLER_SITE = $(TOPDIR)/package/pinController/driver
PINCONTROLLER_SITE_METHOD = local
PINCONTROLLER_LICENSE = GPLv3+

PINCONTROLLER_DEPENDENCIES = linux

define PINCONTROLLER_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define PINCONTROLLER_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef



$(eval $(generic-package))
