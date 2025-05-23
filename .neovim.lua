-- I set those per project
-- This is an example of how you can use the kind of the same
-- Its not exact and you might do it differently, it's somewhat incomplete
-- debuging should be like qt-creator
Plug('neovim/nvim-lspconfig')
Plug('williamboman/mason.nvim')
Plug('williamboman/mason-lspconfig.nvim')
Plug('mfussenegger/nvim-dap')
Plug('mfussenegger/nvim-dap-python')
Plug('nvim-neotest/nvim-nio')
Plug('rcarriga/nvim-dap-ui')
Plug('mfussenegger/nvim-lint')
Plug('Vimjas/vim-python-pep8-indent')

vim.api.nvim_set_keymap('n', 'å', ":lua require'dap'.toggle_breakpoint()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ä', ":lua require'dap'.step_into()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ö', ":lua require'dap'.step_over()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'æ', ":lua require'dap'.continue()<CR>", { noremap = true, silent = true })
vim.api.nvim_set_keymap('n', 'ø', ":lua require'dapui'.toggle()<CR>", { noremap = true, silent = true })

-- See
-- https://sourceware.org/gdb/current/onlinedocs/gdb.html/Interpreters.html
-- https://sourceware.org/gdb/current/onlinedocs/gdb.html/Debugger-Adapter-Protocol.html
-- Require DAP and DAP-Python
-- Use qtcreator to debug c++ part
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

require("dapui").setup()

local nvim_lsp = require('lspconfig')
nvim_lsp.clangd.setup{}
require("mason").setup()
require("mason-lspconfig").setup()

vim.o.shiftwidth = 4
vim.api.nvim_create_autocmd("FileType", {
  pattern = "python",
  callback = function()
    vim.opt_local.expandtab = true
    vim.opt_local.shiftwidth = 4
    vim.opt_local.softtabstop = 4
  end,
})

function ToggleShiftwidth()
  if vim.g.current_shiftwidth == 2 then
    vim.g.current_shiftwidth = 4
  else
    vim.g.current_shiftwidth = 2
  end
  vim.cmd('set shiftwidth=' .. vim.g.current_shiftwidth)
  print('Shiftwidth set to ' .. vim.g.current_shiftwidth)
end
-- The style is meant to keep the code narrow, never let it over 80-100
-- With cpplint --filter=-whitespace/braces,-whitespace/newline

require('lint').linters_by_ft = {
  sh = {'shellcheck'}, -- Ensure you have shellcheck installed
  python = {'pylint', 'bandit', 'ruff', 'pydocstyle', 'mypy', 'flake8'}, -- Ensure these are installed
  cmake = {'cmakelint'},
  cpp = {'cppcheck', 'cpplint', 'flawfinder', 'clangtidy'},
}

-- build cppcheck-2.6.3 or you are on you're own with new version
-- .local/share/nvim/plugged/nvim-lint/lua/lint/linters/cppcheck.lua
-- cppcheck <= 1.84 doesn't support {column} so the start_col group is ambiguous
local pattern = [[([^:]*):(%d*):([^:]*): %[([^%]\]*)%] ([^:]*): (.*)]]
local groups = { "file", "lnum", "col", "code", "severity", "message" }
local severity_map = {
  ["error"] = vim.diagnostic.severity.ERROR,
  ["warning"] = vim.diagnostic.severity.WARN,
  ["performance"] = vim.diagnostic.severity.WARN,
  ["style"] = vim.diagnostic.severity.INFO,
  ["information"] = vim.diagnostic.severity.INFO,
}

return {
  cmd = "cppcheck",
  stdin = false,
  args = {
    "--enable=warning,style,performance,information",
    function()
      if vim.bo.filetype == "cpp" then
        return "--language=c++"
      else
        return "--language=c"
      end
    end,
    "--inline-suppr",
    "--quiet",
    function()
      if vim.fn.isdirectory("build") == 1 then
        return "--cppcheck-build-dir=build"
      else
        return nil
      end
    end,
    "--template={file}:{line}:{column}: [{id}] {severity}: {message}",
    --"--check-level=exhaustive", -- pip cppcheck-wheel 1.4.7 only
    "--force",  --cppcheck-2.6 only
    "--library=qt",
    "--suppress=unmatchedSuppression", -- pip cppcheck-wheel 1.4.7 only
    "--suppress=unusedStructMember",
    "--addon=cert", -- works with cppcheck-2.6 only
    "--addon=y2038", -- works with cppcheck-2.6 only
    "--addon=threadsafety", -- works with cppcheck-2.6 only
    "--addon=.cppcheck-addons/misra.json", --cppcheck-2.6 only
  },
  stream = "stderr",
  parser = require("lint.parser").from_pattern(pattern, groups, severity_map, { ["source"] = "cppcheck" }),
}

-- .local/share/nvim/plugged/nvim-lint/lua/lint/linters/cpplint.lua
-- path/to/file:line:  message  [code] [code_id]
local pattern = '([^:]+):(%d+):  (.+)  (.+)'
local groups = { 'file', 'lnum', 'message', 'code'}

return {
  cmd = 'cpplint',
  stdin = false,
  args = {
    "--filter=-build/include_subdir",
  },
  ignore_exitcode = true,
  stream = 'stderr',
  parser = require('lint.parser').from_pattern(pattern, groups, nil, {
    ['source'] = 'cpplint',
    ['severity'] = vim.diagnostic.severity.WARN,
  }),
}

