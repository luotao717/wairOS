#
# Copyright (C) 2008-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/image.mk

JFFS2_BLOCKSIZE = 64k 128k 256k

define imgname
$(BIN_DIR)/$(IMG_PREFIX)-$(2)-$(patsubst jffs2-%,jffs2,$(patsubst squashfs-%,squashfs,$(1)))
endef

define rootfs_align
$(patsubst %-256k,0x40000,$(patsubst %-128k,0x20000,$(patsubst %-64k,0x10000,$(patsubst squashfs-%,0x4,$(1)))))
endef

define sysupname
$(call imgname,$(1),$(2))-sysupgrade.bin
endef

define factoryname
$(call imgname,$(1),$(2))-factory.bin
endef

COMMA:=,

define mkcmdline
$(if $(1),board=$(1) )$(if $(2),console=$(2)$(COMMA)$(3))
endef

SINGLE_PROFILES:=

define SingleProfile
  define Image/Build/Profile/$(3)
	$$(call Image/Build/Template/$(2)/$$(1),$(1),$(4),$$(call mkcmdline,$(5),$(6),$(7)),$(8),$(9),$(10),$(11),$(12))
  endef
  SINGLE_PROFILES += $(3)
endef

define MultiProfile
  define Image/Build/Profile/$(1)
	$(foreach p,$(2),
		$$(call Image/Build/Profile/$p,$$(1))
	)
  endef
endef

LOADER_MAKE := $(NO_TRACE_MAKE) -C lzma-loader KDIR=$(KDIR)

KDIR_TMP:=$(KDIR)/tmp
VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux
UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage
fs_squash:=squashfs-only
fs_64k:=64k
fs_64kraw:=64kraw
fs_128k:=128k
fs_256k:=256k
ifeq ($(CONFIG_TARGET_ROOTFS_INITRAMFS),y)
	fs_squash:=initramfs
	fs_64k:=initramfs
	fs_64kraw:=initramfs
	fs_128k:=initramfs
	fs_256k:=initramfs
	VMLINUX:=$(BIN_DIR)/$(IMG_PREFIX)-vmlinux-initramfs
	UIMAGE:=$(BIN_DIR)/$(IMG_PREFIX)-uImage-initramfs
endif

define CompressLzma
  $(STAGING_DIR_HOST)/bin/lzma e $(1) -lc1 -lp2 -pb2 $(3) $(2)
endef

define PatchKernelLzma
	cp $(KDIR)/vmlinux $(KDIR_TMP)/vmlinux-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR_TMP)/vmlinux-$(1) "$(strip $(2))"
	$(call CompressLzma,$(KDIR_TMP)/vmlinux-$(1),$(KDIR_TMP)/vmlinux-$(1).bin.lzma,$(3))
endef

define PatchKernelGzip
	cp $(KDIR)/vmlinux $(KDIR_TMP)/vmlinux-$(1)
	$(STAGING_DIR_HOST)/bin/patch-cmdline $(KDIR_TMP)/vmlinux-$(1) "$(strip $(2))"
	gzip -9 -c $(KDIR_TMP)/vmlinux-$(1) > $(KDIR_TMP)/vmlinux-$(1).bin.gz
endef

define MkuImage
	mkimage -A mips -O linux -T kernel -a 0x80060000 -C $(1) $(2) \
		-e 0x80060000 -n 'MIPS OpenWrt Linux-$(LINUX_VERSION)' \
		-d $(3) $(4)
endef

define MkuImageLzma
	$(call PatchKernelLzma,$(1),$(2),$(3))
	$(call MkuImage,lzma,,$(KDIR_TMP)/vmlinux-$(1).bin.lzma,$(KDIR_TMP)/vmlinux-$(1).uImage)
endef

define MkuImageLzma/initramfs
	$(call PatchKernelLzma,$(1),$(2))
	$(call MkuImage,lzma,,$(KDIR_TMP)/vmlinux-$(1).bin.lzma,$(call imgname,initramfs,$(1))-uImage.bin)
endef

define MkuImageGzip
	$(call PatchKernelGzip,$(1),$(2))
	$(call MkuImage,gzip,,$(KDIR_TMP)/vmlinux-$(1).bin.gz,$(KDIR_TMP)/vmlinux-$(1).uImage)
endef

define MkuImageGzip/initramfs
	$(call PatchKernelGzip,$(1),$(2))
	$(call MkuImage,gzip,,$(KDIR_TMP)/vmlinux-$(1).bin.gz,$(call imgname,initramfs,$(1))-uImage.bin)
endef

define MkuImageOKLI
	$(call MkuImage,lzma,-M 0x4f4b4c49,$(KDIR)/vmlinux.bin.lzma,$(KDIR_TMP)/vmlinux-$(1).okli)
endef

define CatFiles
	if [ `stat -c%s "$(1)"` -gt $(2) ]; then \
		echo "Warning: $(1) is too big"; \
	else if [ `stat -c%s $(3)` -gt $(4) ]; then \
		echo "Warning: $(3) is too big"; \
	else \
		( dd if=$(1) bs=$(2) conv=sync; dd if=$(3) ) > $(5); \
	fi; fi
endef

define Sysupgrade/KR
	$(call CatFiles,$(2),$(3),$(KDIR)/root.$(1),$(4),$(call sysupname,$(1),$(5)))
endef

define Sysupgrade/KRuImage
	$(call CatFiles,$(KDIR_TMP)/vmlinux-$(2).uImage,$(3),$(KDIR)/root.$(1),$(4),$(call sysupname,$(1),$(2)))
endef

define Sysupgrade/RKuImage
	$(call CatFiles,$(KDIR)/root.$(1),$(4),$(KDIR_TMP)/vmlinux-$(2).uImage,$(3),$(call sysupname,$(1),$(2)))
endef

define Image/BuildLoader
	-rm -rf $(KDIR)/lzma-loader
	$(LOADER_MAKE) LOADER=loader-$(1).$(2) KERNEL_CMDLINE="$(3)"\
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		LOADER_DATA="$(KDIR)/vmlinux.bin.lzma" BOARD="$(1)" \
		compile loader.$(2)
endef

define Image/BuildLoaderAlone
	-rm -rf $(KDIR)/lzma-loader
	$(LOADER_MAKE) LOADER=loader-$(1).$(2) KERNEL_CMDLINE="$(3)" \
		LZMA_TEXT_START=0x80a00000 LOADADDR=0x80060000 \
		BOARD="$(1)" FLASH_OFFS=$(4) FLASH_MAX=$(5) \
		compile loader.$(2)
endef

define Build/Clean
	$(LOADER_MAKE) clean
endef

wafi_r1001_mtdlayout_8M=mtdparts=spi0.0:128(u-boot)ro,896k(kernel),7104k(rootfs),64k(art),8000@0x20000(firmware)

define Image/BuildKernel
	cp $(KDIR)/vmlinux.elf $(VMLINUX).elf
	cp $(KDIR)/vmlinux $(VMLINUX).bin
	dd if=$(KDIR)/vmlinux.bin.lzma of=$(VMLINUX).lzma bs=65536 conv=sync
	dd if=$(KDIR)/vmlinux.bin.gz of=$(VMLINUX).gz bs=65536 conv=sync
	$(call MkuImage,gzip,,$(KDIR)/vmlinux.bin.gz,$(UIMAGE)-gzip.bin)
	$(call MkuImage,lzma,,$(KDIR)/vmlinux.bin.lzma,$(UIMAGE)-lzma.bin)
	cp $(KDIR)/loader-generic.elf $(VMLINUX)-lzma.elf
	-mkdir -p $(KDIR_TMP)
	$(call Image/Build/Initramfs)
endef


define Image/Build/Ath
	$(call Sysupgrade/$(7),$(1),$(2),$(5),$(6))
	if [ -e "$(call sysupname,$(1),$(2))" ]; then \
		dd if=$(KDIR_TMP)/vmlinux-$(2).uImage \
			of=$(call imgname,kernel,$(2)).bin bs=64k conv=sync; \
		dd if=$(KDIR)/root.$(1) \
			of=$(call imgname,$(1),$(2)-rootfs).bin bs=128k conv=sync; \
	fi
endef

define Image/Build/AthGzip
	$(call MkuImageGzip,$(2),$(3) $(4))
	$(call Image/Build/Ath,$(1),$(2),$(3),$(4),$(5),$(6),$(7))
endef

define Image/Build/AthGzip/initramfs
	$(call MkuImageGzip/initramfs,$(2),$(3) $(4))
endef

define Image/Build/AthLzma
	$(call MkuImageLzma,$(2),$(3) $(4))
	$(call Image/Build/Ath,$(1),$(2),$(3),$(4),$(5),$(6),$(7))
endef

define Image/Build/AthLzma/initramfs
	$(call MkuImageLzma/initramfs,$(2),$(3) $(4))
endef

define Image/Build/MyLoader
	$(call PatchKernelLzma,$(2),$(3))
	-$(STAGING_DIR_HOST)/bin/mkmylofw -B $(2) -s $(4) \
		-p0x030000:0xe0000:al:0x80060000:kernel:$(KDIR_TMP)/vmlinux-$(2).bin.lzma \
		-p0x110000:0:::rootfs:$(KDIR)/root.$(1) \
		$(call imgname,$(1),$(2))-$(5)-factory.img
endef

define Image/Build/TPLINK-LZMA
	$(call PatchKernelLzma,$(2),$(3))
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(4) -W $(5) -F $(6) -N OpenWrt -V $(REVISION) $(7) \
		-k $(KDIR_TMP)/vmlinux-$(2).bin.lzma \
		-r $(KDIR)/root.$(1) \
		-a $(call rootfs_align,$(1)) -j \
		-o $(call factoryname,$(1),$(2))
	-$(STAGING_DIR_HOST)/bin/mktplinkfw \
		-H $(4) -W $(5) -F $(6) -N OpenWrt -V $(REVISION) $(7) -s \
		-k $(KDIR_TMP)/vmlinux-$(2).bin.lzma \
		-r $(KDIR)/root.$(1) \
		-a $(call rootfs_align,$(1)) -j \
		-o $(call sysupname,$(1),$(2))
endef

define Image/Build/Template/initramfs/initramfs
	$(call Image/Build/$(1)/initramfs,initramfs,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/all/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/all/jffs2-64k
	$(call Image/Build/$(1),jffs2-64k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/all/jffs2-128k
	$(call Image/Build/$(1),jffs2-128k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/all/jffs2-256k
	$(call Image/Build/$(1),jffs2-256k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/squashfs-only/loader
	$(call Image/Build/$(1)/loader,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/squashfs-only/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64k/loader
	$(call Image/Build/$(1)/loader,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64k/squashfs
	$(call Image/Build/$(1),squashfs-64k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64k/jffs2-64k
	$(call Image/Build/$(1),jffs2-64k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64kraw/loader
	$(call Image/Build/$(1)/loader,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64kraw/squashfs
	$(call Image/Build/$(1),squashfs-raw,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/64kraw/jffs2-64k
	$(call Image/Build/$(1),jffs2-64k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/128k/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/128k/jffs2-128k
	$(call Image/Build/$(1),jffs2-128k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/256k/squashfs
	$(call Image/Build/$(1),squashfs,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef

define Image/Build/Template/256k/jffs2-256k
	$(call Image/Build/$(1),jffs2-256k,$(2),$(3),$(4),$(5),$(6),$(7),$(8))
endef


$(eval $(call SingleProfile,TPLINK-LZMA,$(fs_64kraw),WAFI_R1001_8M,wafi-r1001-8M,WAFI-R1001,ttyS0,115200,0x08410008,1,8Mlzma))

$(eval $(call MultiProfile,WAFI_R1001,WAFI_R1001_8M))

define Image/Build/squashfs
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-raw
	cp $(KDIR)/root.squashfs $(KDIR)/root.squashfs-64k
	$(STAGING_DIR_HOST)/bin/padjffs2 $(KDIR)/root.squashfs-64k 64
	cp $(KDIR)/root.squashfs-64k $(BIN_DIR)/$(IMG_PREFIX)-root.squashfs-64k
	$(call prepare_generic_squashfs,$(KDIR)/root.squashfs)
endef

define Image/Build/Initramfs
	$(call Image/Build/Profile/$(PROFILE),initramfs)
endef

define Image/Prepare
	rm -rf $(TARGET_DIR)/usr/lib/opkg
	rm -rf $(TARGET_DIR)/usr/lib/ikpg
	# Create an empty opkg dir, to make it easier to use opkg on the
	# Fonera.
	mkdir -p $(TARGET_DIR)/usr/lib/opkg
	#for a in `grep usr/bin/lua $(TARGET_DIR)/etc/fonstated/*| cut -d: -f1`; do \
	#	${STAGING_DIR_HOST}/bin/luac -s -o $$$${a}.tmp $$$$a; \
	#	echo '#!/usr/bin/lua' > $$$$a; \
	#	cat $$$${a}.tmp >> $$$$a; \
	#	rm $$$${a}.tmp; \
	#done
	#chmod a+x  $(TARGET_DIR)/etc/fonstated/*
	#wuxiaobo add for compat old version
	mkdir -p $(TARGET_DIR)/usr/lib/opkg/ipk_reserve
#	cp $(BIN_DIR)/packages/luci-theme-r3000_ODM_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_ODM.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_Momo_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_Momo.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_GSD_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_GSD.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_ODM_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_ODM.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_RL_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_RL.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_Pakistan_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_Pakistan.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_Turkey_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_Turkey.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_Vietnam_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_Vietnam.ipk	
#	cp $(BIN_DIR)/packages/luci-theme-r3000_Jiesai_0.10+svn7109-1_ath79.ipk $(TARGET_DIR)/usr/lib/opkg/ipk_reserve/r3000_Jiesai.ipk	

	gzip -9 -c $(KDIR)/vmlinux > $(KDIR)/vmlinux.bin.gz
	$(call CompressLzma,$(KDIR)/vmlinux,$(KDIR)/vmlinux.bin.lzma)
	$(call Image/BuildLoader,generic,elf)
	$(call Image/Build/Profile/$(if $(CONFIG_IB),Default,$(PROFILE)),loader)
endef

define Image/Build
	$(call Image/Build/$(1))
	dd if=$(KDIR)/root.$(1) of=$(BIN_DIR)/$(IMG_PREFIX)-root.$(1) bs=128k conv=sync
	$(call Image/Build/Profile/$(PROFILE),$(1))
endef

$(eval $(call BuildImage))
