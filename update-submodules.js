#!/usr/bin/env node

/**
 * update-submodules.js
 * Recursively updates all Git submodules (and their nested submodules)
 * to the latest commit on their tracked remote branch.
 *
 * Usage:
 *   node update-submodules.js [repo-path] [options]
 *
 * Options:
 *   --dry-run     Show what would be updated without making changes
 *   --branch <b>  Default branch to pull if submodule has no tracked branch (default: main)
 *   --depth <n>   Max recursion depth (default: unlimited)
 *   --verbose     Show full git output
 *   --no-color    Disable colored output
 *
 * Examples:
 *   node update-submodules.js
 *   node update-submodules.js /path/to/repo --dry-run
 *   node update-submodules.js --branch master --depth 3 --verbose
 */

const { execSync, spawnSync } = require("child_process");
const path = require("path");
const fs = require("fs");

// ─── CLI argument parsing ────────────────────────────────────────────────────

const args = process.argv.slice(2);

const options = {
  repoPath: process.cwd(),
  dryRun: false,
  defaultBranch: "main",
  maxDepth: Infinity,
  verbose: false,
  color: true,
};

for (let i = 0; i < args.length; i++) {
  const arg = args[i];
  if (arg === "--dry-run") options.dryRun = true;
  else if (arg === "--verbose") options.verbose = true;
  else if (arg === "--no-color") options.color = false;
  else if (arg === "--branch") options.defaultBranch = args[++i];
  else if (arg === "--depth") options.maxDepth = parseInt(args[++i], 10);
  else if (!arg.startsWith("--")) options.repoPath = path.resolve(arg);
}

// ─── Colour helpers ──────────────────────────────────────────────────────────

const C = {
  reset: options.color ? "\x1b[0m" : "",
  bold: options.color ? "\x1b[1m" : "",
  dim: options.color ? "\x1b[2m" : "",
  green: options.color ? "\x1b[32m" : "",
  yellow: options.color ? "\x1b[33m" : "",
  cyan: options.color ? "\x1b[36m" : "",
  red: options.color ? "\x1b[31m" : "",
  magenta: options.color ? "\x1b[35m" : "",
  blue: options.color ? "\x1b[34m" : "",
};

// ─── Logging ─────────────────────────────────────────────────────────────────

const indent = (depth) => "  ".repeat(depth);

function log(depth, symbol, color, msg) {
  console.log(`${indent(depth)}${color}${symbol} ${msg}${C.reset}`);
}

function info(depth, msg) {
  log(depth, "›", C.cyan, msg);
}
function success(depth, msg) {
  log(depth, "✔", C.green, msg);
}
function warn(depth, msg) {
  log(depth, "⚠", C.yellow, msg);
}
function error(depth, msg) {
  log(depth, "✘", C.red, msg);
}
function header(depth, msg) {
  log(depth, "▸", C.bold + C.magenta, msg);
}
function verbose(depth, msg) {
  if (options.verbose) log(depth, " ", C.dim, msg);
}

// ─── Git helpers ─────────────────────────────────────────────────────────────

/**
 * Run a git command in the given directory.
 * Returns { stdout, stderr, status }.
 */
function git(cwd, ...gitArgs) {
  const result = spawnSync("git", gitArgs, {
    cwd,
    encoding: "utf8",
    env: { ...process.env, GIT_TERMINAL_PROMPT: "0" },
  });
  return {
    stdout: (result.stdout || "").trim(),
    stderr: (result.stderr || "").trim(),
    status: result.status,
    ok: result.status === 0,
  };
}

/**
 * Check whether a directory is the root of a git repository.
 */
function isGitRepo(dir) {
  return fs.existsSync(path.join(dir, ".git"));
}

/**
 * Parse .gitmodules and return an array of submodule objects:
 *   { name, path, url, branch? }
 */
function parseGitmodules(repoDir) {
  const gmPath = path.join(repoDir, ".gitmodules");
  if (!fs.existsSync(gmPath)) return [];

  const raw = fs.readFileSync(gmPath, "utf8");
  const submodules = [];
  let current = null;

  for (const rawLine of raw.split("\n")) {
    const line = rawLine.trim();
    if (!line || line.startsWith("#")) continue;

    const sectionMatch = line.match(/^\[submodule\s+"(.+)"\]$/);
    if (sectionMatch) {
      current = { name: sectionMatch[1], path: null, url: null, branch: null };
      submodules.push(current);
      continue;
    }

    if (!current) continue;

    const kvMatch = line.match(/^(\w+)\s*=\s*(.+)$/);
    if (kvMatch) {
      const [, key, value] = kvMatch;
      if (key === "path") current.path = value;
      else if (key === "url") current.url = value;
      else if (key === "branch") current.branch = value;
    }
  }

  return submodules.filter((s) => s.path);
}

/**
 * Resolve the branch to pull for a submodule.
 * Priority: .gitmodules branch > remote HEAD > defaultBranch option.
 */
function resolveBranch(submoduleDir, declaredBranch) {
  if (declaredBranch) return declaredBranch;

  // Ask the remote what its HEAD branch is
  const result = git(submoduleDir, "remote", "show", "origin");
  if (result.ok) {
    const match = result.stdout.match(/HEAD branch:\s+(\S+)/);
    if (match) return match[1];
  }

  return options.defaultBranch;
}

// ─── Statistics ───────────────────────────────────────────────────────────────

const stats = {
  updated: 0,
  upToDate: 0,
  skipped: 0,
  failed: 0,
  total: 0,
};

// ─── Core recursive function ──────────────────────────────────────────────────

/**
 * Update all submodules inside `repoDir`, recursing into each.
 * Returns true if any submodule ref changed (so the caller can stage the parent).
 *
 * @param {string} repoDir   Absolute path to the git repo being processed
 * @param {number} depth     Current recursion depth (0 = root)
 */
function updateSubmodules(repoDir, depth = 0) {
  if (depth > options.maxDepth) {
    warn(depth, `Max depth (${options.maxDepth}) reached — stopping here.`);
    return;
  }

  const submodules = parseGitmodules(repoDir);

  if (submodules.length === 0) {
    verbose(depth, "No submodules found.");
    return;
  }

  for (const sub of submodules) {
    const subDir = path.resolve(repoDir, sub.path);
    stats.total++;

    header(depth, `${sub.name}  ${C.dim}(${sub.path})${C.reset}`);

    // ── Ensure the submodule directory is initialised ──────────────────────

    if (!fs.existsSync(subDir) || !isGitRepo(subDir)) {
      info(
        depth + 1,
        "Submodule not initialised — running git submodule update --init",
      );
      if (!options.dryRun) {
        const init = git(
          repoDir,
          "submodule",
          "update",
          "--init",
          "--",
          sub.path,
        );
        if (!init.ok) {
          error(depth + 1, `Failed to init submodule: ${init.stderr}`);
          stats.failed++;
          continue;
        }
        verbose(depth + 1, init.stdout);
      }
    }

    if (!fs.existsSync(subDir)) {
      warn(depth + 1, "Directory still missing after init — skipping.");
      stats.skipped++;
      continue;
    }

    // ── Fetch latest from remote ───────────────────────────────────────────

    info(depth + 1, "Fetching from origin…");
    if (!options.dryRun) {
      const fetch = git(subDir, "fetch", "--prune", "origin");
      if (!fetch.ok) {
        warn(
          depth + 1,
          `Fetch failed (${fetch.stderr}) — proceeding with local data.`,
        );
      } else {
        verbose(depth + 1, fetch.stderr || "fetch ok");
      }
    }

    // ── Determine target branch ────────────────────────────────────────────

    const branch = resolveBranch(subDir, sub.branch);
    info(depth + 1, `Target branch: ${C.bold}${branch}${C.reset}`);

    // ── What does the remote tip look like right now? ──────────────────────
    // This is the commit we WANT to be at. Compare against current HEAD.

    const remoteRef = `origin/${branch}`;
    const remoteTip = git(subDir, "rev-parse", remoteRef).stdout;

    if (!remoteTip) {
      warn(
        depth + 1,
        `Cannot resolve ${remoteRef} — has this branch been pushed?`,
      );
      stats.skipped++;
      continue;
    }

    // ── Record current HEAD (what the parent repo is pinned to) ───────────

    const beforeHash = git(subDir, "rev-parse", "HEAD").stdout;

    // ── Dry-run: just report what would change ─────────────────────────────

    if (options.dryRun) {
      if (beforeHash === remoteTip) {
        success(depth + 1, `Already up to date (${remoteTip.slice(0, 8)})`);
        stats.upToDate++;
      } else {
        success(
          depth + 1,
          `Would update  ${C.dim}${beforeHash.slice(0, 8)}${C.reset} → ${C.bold}${C.green}${remoteTip.slice(0, 8)}${C.reset}  (dry-run)`,
        );
        stats.updated++;
      }
      updateSubmodules(subDir, depth + 1);
      continue;
    }

    // ── Checkout the branch, then hard-reset to origin/<branch> ───────────
    // Using reset --hard is the reliable way to move the submodule to the
    // exact remote tip regardless of detached HEAD state or local commits.

    const checkout = git(subDir, "checkout", "-B", branch, remoteRef);
    if (!checkout.ok) {
      // Fallback: try a plain checkout then reset
      const co2 = git(subDir, "checkout", branch);
      if (!co2.ok) {
        error(depth + 1, `Cannot checkout '${branch}': ${co2.stderr}`);
        stats.failed++;
        continue;
      }
      const reset = git(subDir, "reset", "--hard", remoteRef);
      if (!reset.ok) {
        error(depth + 1, `reset --hard failed: ${reset.stderr}`);
        stats.failed++;
        continue;
      }
      verbose(depth + 1, reset.stdout);
    } else {
      verbose(depth + 1, checkout.stdout || checkout.stderr);
    }

    const afterHash = git(subDir, "rev-parse", "HEAD").stdout;

    // ── Stage the updated submodule ref in the parent repo ────────────────
    // Without this the parent still records the old commit pointer.

    const addResult = git(repoDir, "add", sub.path);
    if (!addResult.ok) {
      warn(
        depth + 1,
        `Could not stage updated ref in parent: ${addResult.stderr}`,
      );
    } else {
      verbose(depth + 1, `Staged ${sub.path} in parent repo`);
    }

    // ── Report ─────────────────────────────────────────────────────────────

    if (beforeHash === afterHash) {
      success(depth + 1, `Already up to date (${afterHash.slice(0, 8)})`);
      stats.upToDate++;
    } else {
      success(
        depth + 1,
        `Updated  ${C.dim}${beforeHash.slice(0, 8)}${C.reset} → ${C.bold}${C.green}${afterHash.slice(0, 8)}${C.reset}`,
      );
      stats.updated++;
    }

    // ── Recurse into this submodule's own submodules ───────────────────────

    updateSubmodules(subDir, depth + 1);
  }
}

// ─── Entry point ─────────────────────────────────────────────────────────────

function main() {
  console.log();
  console.log(
    `${C.bold}${C.blue}╔══════════════════════════════════════════╗${C.reset}`,
  );
  console.log(
    `${C.bold}${C.blue}║   Git Submodule Recursive Updater        ║${C.reset}`,
  );
  console.log(
    `${C.bold}${C.blue}╚══════════════════════════════════════════╝${C.reset}`,
  );
  console.log();

  if (!isGitRepo(options.repoPath)) {
    error(0, `Not a git repository: ${options.repoPath}`);
    process.exit(1);
  }

  info(0, `Repository : ${C.bold}${options.repoPath}${C.reset}`);
  info(0, `Default branch : ${C.bold}${options.defaultBranch}${C.reset}`);
  if (options.dryRun) warn(0, "DRY RUN — no changes will be made");
  if (options.maxDepth !== Infinity)
    info(0, `Max depth  : ${options.maxDepth}`);
  console.log();

  const startTime = Date.now();

  updateSubmodules(options.repoPath, 0);

  const elapsed = ((Date.now() - startTime) / 1000).toFixed(2);

  console.log();
  console.log(
    `${C.bold}${C.blue}─────────────────────────────────────────${C.reset}`,
  );
  console.log(`${C.bold}Summary${C.reset}`);
  console.log(`  ${C.green}✔ Updated   : ${stats.updated}${C.reset}`);
  console.log(`  ${C.cyan}· Up to date: ${stats.upToDate}${C.reset}`);
  console.log(`  ${C.yellow}⚠ Skipped   : ${stats.skipped}${C.reset}`);
  console.log(`  ${C.red}✘ Failed    : ${stats.failed}${C.reset}`);
  console.log(
    `  ${C.dim}  Total     : ${stats.total}  (${elapsed}s)${C.reset}`,
  );
  console.log();

  if (stats.failed > 0) process.exit(1);
}

main();
