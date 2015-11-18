HELLOWORLD_VERSION = 666
HELLOWORLD_SITE = $(TOPDIR)/package/helloWorld
HELLOWORLD_SITE_METHOD = local
HELLOWORLD_LICENSE = beerware

define HELLOWORLD_BUILD_CMDS
	CC=$(TARGET_CC) $(MAKE) -C $(@D)
endef

define HELLOWORLD_INSTALL_TARGET_CMDS
	prefix=$(TARGET_DIR) $(MAKE) -C $(@D) install
endef

$(eval $(generic-package))
