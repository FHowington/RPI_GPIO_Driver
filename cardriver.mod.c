#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x16894a82, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0xc1a99548, __VMLINUX_SYMBOL_STR(gpiod_unexport) },
	{ 0xa7fe998f, __VMLINUX_SYMBOL_STR(class_unregister) },
	{ 0xb3a50341, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0xb4e862e3, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0xfe4a32c8, __VMLINUX_SYMBOL_STR(gpiod_export) },
	{ 0x6e5b47b6, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0xf91190e7, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0x3b004ba, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x1271abd8, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xf385a1f7, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0x773638c7, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0xf4fa543b, __VMLINUX_SYMBOL_STR(arm_copy_to_user) },
	{ 0x7d11c268, __VMLINUX_SYMBOL_STR(jiffies) },
	{ 0x85825898, __VMLINUX_SYMBOL_STR(add_timer) },
	{ 0x8fdf772a, __VMLINUX_SYMBOL_STR(init_timer_key) },
	{ 0x27bbf221, __VMLINUX_SYMBOL_STR(disable_irq_nosync) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xfcec0987, __VMLINUX_SYMBOL_STR(enable_irq) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "0405596C8C831EAD2D58114");
