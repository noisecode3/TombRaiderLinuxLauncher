-- debuging should be like qt-creator
Plug('neovim/nvim-lspconfig')
Plug('williamboman/mason.nvim')
Plug('williamboman/mason-lspconfig.nvim')
Plug('mfussenegger/nvim-dap')
Plug('nvim-neotest/nvim-nio')
Plug('rcarriga/nvim-dap-ui')

vim.api.nvim_set_keymap('n', 'å', ":lua require'dap'.toggle_breakpoint()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ä', ":lua require'dap'.step_into()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ö', ":lua require'dap'.step_over()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'æ', ":lua require'dap'.continue()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ø', ":lua require'dapui'.toggle()<CR>", { noremap = true, silent = true })

-- See
-- https://sourceware.org/gdb/current/onlinedocs/gdb.html/Interpreters.html
-- https://sourceware.org/gdb/current/onlinedocs/gdb.html/Debugger-Adapter-Protocol.html
require("dap").adapters.gdb = {
    id = 'gdb',
    type = 'executable',
    command = 'gdb',
    args = { '--quiet', '--interpreter=dap' },
}


require("dap").configurations.cpp = {
    {
        name = 'Run executable (GDB)',
        type = 'gdb',
        request = 'launch',
        program = '~/TombRaiderLinuxLauncher/TombRaiderLinuxLauncher',
    }
}


require("dapui").setup()

local nvim_lsp = require('lspconfig')
nvim_lsp.clangd.setup{}
require("mason").setup()
require("mason-lspconfig").setup()


vim.g.current_shiftwidth = 4
