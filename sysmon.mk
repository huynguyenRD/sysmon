SYSMON_VERSION = 1.0
SYSMON_SITE = $(BR2_EXTERNAL_ST_PATH)/package/sysmon/src
SYSMON_SITE_METHOD = local
SYSMON_LICENSE = MIT
SYSMON_DEPENDENCIES = libhal

define SYSMON_BUILD_CMDS
	$(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)
endef

define SYSMON_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/sysmon $(TARGET_DIR)/usr/bin/sysmon
endef

$(eval $(generic-package))