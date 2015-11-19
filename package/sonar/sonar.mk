SONAR_VERSION = 1.0
SONAR_SITE = $(TOPDIR)/package/sonar/driver
SONAR_SITE_METHOD = local
SONAR_LICENSE = GPLv3+

SONAR_DEPENDENCIES = linux

define SONAR_BUILD_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D)
endef

define SONAR_INSTALL_TARGET_CMDS
	$(MAKE) -C $(LINUX_DIR) $(LINUX_MAKE_FLAGS) M=$(@D) modules_install
endef



$(eval $(generic-package))
