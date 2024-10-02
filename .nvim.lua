vim.opt.makeprg = "build.bat";
vim.opt.errorformat = " %#%f(%l\\,%c):\\ %m"

vim.keymap.set("n", "<A-c>", "<cmd>make<CR>");
