-- debuging should be like qt-creator
Plug('neovim/nvim-lspconfig')
Plug('williamboman/mason.nvim')
Plug('williamboman/mason-lspconfig.nvim')
Plug('mfussenegger/nvim-dap')
Plug('mfussenegger/nvim-dap-python')
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
-- Require DAP and DAP-Python
local dap = require("dap")
require("dap-python").setup("python")

-- Python configuration
dap.configurations.python = {
    {
        -- The most basic Python configuration
        name = "Launch file",
        type = "python",
        request = "launch",
        program = "${file}", -- This will launch the current file
        pythonPath = function()
            local venv_path = vim.fn.getenv("VIRTUAL_ENV")
            if venv_path and venv_path ~= vim.NIL and venv_path ~= '' then
                return venv_path .. "/bin/python"
            else
                return "/usr/bin/python3" -- Fallback to system python
            end
        end,
        args = function()
            local input = vim.fn.input("Arguments: ")
            return vim.split(input, " ", true)
        end,
    },
}

-- C++ configuration with GDB
dap.adapters.gdb = {
    type = "executable",
    command = "gdb", -- Ensure GDB is installed and available in your PATH
    args = { "-i", "dap" }
}

dap.configurations.cpp = {
    {
        name = 'Launch executable (GDB)',
        type = 'gdb',
        request = 'launch',
        program = '~/TombRaiderLinuxLauncher/TombRaiderLinuxLauncher',
        stopOnEntry = false, -- or true if you want to stop at the beginning
        cwd = '${workspaceFolder}',
        setupCommands = {
            {
                text = '-enable-pretty-printing', -- Enable pretty-printing for gdb
                description = 'enable pretty printing',
                ignoreFailures = false
            },
        },
        args = {}, -- Pass arguments to the executable
        environment = {}, -- Set environment variables here
    }
}

require("dapui").setup()

local nvim_lsp = require('lspconfig')
nvim_lsp.clangd.setup{}
require("mason").setup()
require("mason-lspconfig").setup()


vim.g.current_shiftwidth = 4
