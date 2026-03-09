# QUANTUM LANGUAGE 🔐⚡

### _Because Python was too easy and C++ wasn't painful enough_

---

> **"Write once, run anywhere, hack everything."**
> — Muhammad Saad Amin, definitely while wearing a hoodie in a dark room

---

## What Is This?

Quantum is a **dynamically-typed scripting language** built from scratch in C++17 by someone who clearly had too much free time and not enough therapy.

It uses the `.sa` file extension. We don't know what `.sa` stands for. **Saad Amin**? **Slightly Ambitious**? **Suspiciously Ambitious**? The world may never know.

You run it like this:

```
quantum script.sa
```

One command. Very cool. Very hacker. Much wow.

---

## Why Does This Exist?

Excellent question. Python exists. JavaScript exists. Bash exists. Rust exists. But apparently none of them were _cybersecurity-ready_ enough, so Saad said:

> _"You know what the world needs? Another scripting language. But this one has `rot13`."_

And thus, Quantum was born.

---

## ✨ "Features"

| Feature                   | What They Said                                 | What It Means                                                    |
| ------------------------- | ---------------------------------------------- | ---------------------------------------------------------------- |
| 🔀 Multi-syntax           | "Python, JS, and C++ syntax work side by side" | The parser is just vibing                                        |
| 🧠 Dynamic Typing         | "No type declarations needed"                  | Types are just suggestions anyway                                |
| 🌳 Tree-walk Interpreter  | "Clean, hackable engine"                       | It's slow but it's _artisanal_                                   |
| 💻 REPL Mode              | "Interactive shell"                            | For when you want to feel like a hacker but only have 10 minutes |
| 🔐 Cybersecurity Builtins | "`xor_bytes`, `rot13`"                         | ROT13. In 2026. Peak security.                                   |
| 🎯 First-class Functions  | "Closures, lambdas, arrow functions"           | Basically just stole JS's homework                               |
| 🏛️ OOP                    | "Classes with inheritance"                     | Because of course it has classes                                 |

---

## The Cybersecurity "Roadmap" 🗺️

The README lists these as **reserved keywords for upcoming features**:

- `scan` — _Network scanning_ (totally legal, we promise)
- `payload` — _"Exploit payload construction"_ (sir this is a GitHub repo)
- `encrypt` / `decrypt` — AES, RSA (coming soon™)
- `hash` — MD5, SHA-256 (MD5 in 2026 👀)

To be fair, `rot13` is already implemented, so the NSA is shaking.

---

## Quick Start

```bash
# Step 1: Question your life choices
git clone https://github.com/SENODROOM/Quantum-Language.git

# Step 2: Build it (pray to CMake gods)
mkdir build && cd build
cmake ..
cmake --build . --config Release

# Step 3: Write your first script
echo 'print("Hello from Quantum!")' > hello.sa

# Step 4: Run it and feel like a hacker
quantum hello.sa
# → Hello from Quantum!

# Step 5: Realize Python would've taken 3 seconds
```

---

## Project Structure

```
Quantum-Language/
├── src/
│   ├── main.cpp          # Where dreams begin
│   ├── Lexer.cpp         # Where strings go to die
│   ├── Parser.cpp        # Where grammar goes to be questioned
│   ├── Interpreter.cpp   # Where bugs are born
│   └── Value.cpp         # Where types go to not exist
├── examples/
│   ├── Python/           # "See, it's just like Python!"
│   ├── C/                # "See, it's just like C!"
│   └── C++/              # "See, it's just like C++!" (It is not)
└── README.md             # This document, but less funny
```

---

## Sample Code: XOR Encryption

```quantum
fn xor_encrypt(text, key):
    return xor_bytes(text, key)

message   = "Hello, Quantum!"
key       = "secret"
encrypted = xor_encrypt(message, key)
decrypted = xor_encrypt(encrypted, key)

print("Encrypted:", to_hex(encrypted))
print("Decrypted:", decrypted)
```

> XOR encryption. The cryptographic equivalent of a bike lock. But hey, it's _built-in_, so that's something.

---

## Frequently Asked Questions

**Q: Is this production-ready?**
A: It has 0 stars on GitHub. You decide.

**Q: Should I use this for real cybersecurity work?**
A: Please don't.

**Q: Why `.sa` extension?**
A: We asked. Saad didn't respond. We assume it's classified.

**Q: Is `rot13` really a cybersecurity feature?**
A: Julius Caesar used it. He was eventually stabbed by his friends, but that's unrelated.

**Q: Will `payload` keyword actually help me build exploits?**
A: It will help GitHub flag your repo.

---

## CLI Reference

```
quantum <file.sa>          Run a Quantum script (and feel powerful)
quantum                    Start REPL (pretend you're in Mr. Robot)
quantum --check <file.sa>  Check syntax (it won't pass, but nice try)
quantum --version          Confirm you installed this
quantum --help             Admit you're lost
quantum --aura             ??? (We're scared to run this)
```

---

## Competitor Analysis 📊

Let's see how Quantum stacks up against the competition:

| Language    | Created By       | Years in Development | Stars    | Has `rot13` |
| ----------- | ---------------- | -------------------- | -------- | ----------- |
| Python      | Guido van Rossum | 4 years              | 60k+     | No          |
| JavaScript  | Brendan Eich     | 10 days              | Universe | No          |
| Rust        | Mozilla          | 6 years              | 90k+     | No          |
| Lua         | PUC-Rio team     | 2 years              | 5k+      | No          |
| **Quantum** | **1 guy**        | **3 weeks**          | **0**    | **✅ YES**  |

**Winner: Quantum.** (On the `rot13` metric exclusively.)

---

## Real User Testimonials™

> _"I tried to use it for penetration testing. The only thing that got penetrated was my patience."_
> — Anonymous Security Researcher

> _"The multi-syntax support is incredible. I can write bad code in three languages simultaneously."_
> — A Confused Developer

> _"I ran `quantum --aura` and my computer started playing lo-fi hip hop. I don't know what happened."_
> — @definitely_a_real_user

> _"Finally, a language where I can write Python-style code that runs at C++ speeds."_
> — Someone who hasn't benchmarked a tree-walk interpreter

> _"My `payload` keyword isn't implemented yet but I appreciate the commitment to the bit."_
> — Future cybercriminal, probably

---

## Error Messages You Will Definitely See

```
QuantumError: Unexpected token at line 1
  → You typed something

QuantumError: Undefined variable 'x'
  → You forgot dynamic typing doesn't mean magic

QuantumError: Stack overflow in recursive function
  → The irony of a language named Quantum having classical problems

QuantumError: Cannot XOR a string with your disappointment
  → Personal attack from the interpreter

QuantumError: rot13 applied twice returns original string
  → This is a feature, not a bug. This is literally how rot13 works.
```

---

## The `.sa` Extension: A Deep Investigation 🕵️

We have spent considerable time researching what `.sa` stands for. Our findings:

- **S**aad **A**min ← most likely
- **S**cripting **A**bomination ← unfair but considered
- **S**omewhat **A**mbiguous ← accurate
- **S**till **A**nother language extension ← we checked, `.py`, `.js`, `.rb`, `.go`, `.rs`, `.lua` were all taken
- **S**ecret **A**genda ← given the `payload` keyword, plausible
- **S**yntax **A**narchy ← Python + JS + C++ in one file suggests yes

**Official conclusion:** Unknown. Classified. `.sa`.

---

## Performance Benchmarks 🏎️

We ran Quantum against Python on a fibonacci(35) test:

```
Python 3.12:     1.2 seconds
Quantum v1.0.0:  [still running]
Node.js:         0.4 seconds
Quantum v1.0.0:  [still running]
Rust:            0.002 seconds
Quantum v1.0.0:  [still running]
```

_Tree-walk interpreters are known for their warmth, character, and charm. Speed is for people without personality._

---

## Security Advisory ⚠️

The following Quantum code has been flagged as a potential threat:

```quantum
message = "Hello World"
print(rot13(message))
# Output: "Uryyb Jbeyq"
# Status: ENCRYPTED. Unbreakable. Do not share.
```

If you receive a message encoded with Quantum's `rot13`, please apply `rot13` again to decrypt it. This is the entire security model.

**CVE Status:** None filed yet. Give it time.

---

## Comparison: Writing Hello World

**In Python:**

```python
print("Hello World")
```

**In Quantum:**

```quantum
print("Hello World")
```

**Identical.** This raises the question of what exactly was built here, but we respect the hustle.

---

## What The `--aura` Flag Actually Does

Nobody knows. The README mentions it shows a "project achievement board." We ran it. We saw things. We don't talk about it.

What we _can_ say is that it's the only CLI flag in any programming language runtime that references **vibes** as a metric. For that alone, Quantum deserves to exist.

---

## Things Quantum Can Do That Python Can't 🏆

After extensive research, we found **one**:

- `quantum --aura`

Python cannot do this. Advantage: Quantum.

---

## The OOP System, Explained 🏛️

Quantum supports classes with inheritance. Here's a real example from the README:

```quantum
class Animal:
    fn init(name, sound):
        self.name  = name
        self.sound = sound

class Dog extends Animal:
    fn fetch(item):
        return self.name + " fetches the " + item

dog = Dog("Rex", "Woof")
print(dog.speak())
print(dog.fetch("ball"))
```

Riveting. A dog fetching a ball. This is the flagship cybersecurity demo.

We assume the next example is:

```quantum
dog = Dog("Rex", "Woof")
dog.fetch("your_passwords.txt")
```

---

## Quantum's Multi-Syntax Support In Practice 🔀

The README proudly claims you can mix Python, JavaScript, and C++ syntax freely. Here's what that looks like in the wild:

```quantum
// C++ style comment
const x = 10        # JavaScript const, Python comment
if x > 5:           # Python colon
{                   # C++ brace (why not)
    console.log(x)  # JavaScript console
}
print(x)            # Python print
```

Is this a feature? Is this a cry for help? Is this what happens when you learn three languages at once and refuse to commit? **Yes. Yes to all of it.**

---

## Quantum Through The Ages 📅

**Week 1:** Lexer written. Tokens tokenizing. Saad feeling unstoppable.

**Week 2:** Parser done. Tree walking. `rot13` added at 2am for reasons that made sense at the time.

**Week 3:** OOP, closures, REPL, cybersecurity builtins, VS Code extension, CMake build system, full documentation, examples in three syntax styles, v1.0.0 tagged and shipped to GitHub.

**Week 4:** 0 stars. Saad stares at the ceiling.

**Week 5:** Some guy on the internet writes a roast README. Saad is unsure if this is better or worse than 0 stars.

---

## Things The README Says vs. Reality 🔍

**README says:** _"Quantum is purpose-built for cybersecurity tooling"_
**Reality:** It has `rot13` and `xor`. Your WiFi password is safe.

**README says:** _"Write it like Python, JavaScript, or C/C++ and it will just work"_
**Reality:** It will work until it doesn't, at which point you'll get a stack trace in three syntaxes.

**README says:** _"Clean, hackable execution engine"_
**Reality:** Tree-walk interpreters are the minivans of language runtimes. Reliable. Lovable. Slow.

**README says:** _"Rich Standard Library"_
**Reality:** It has `random`, `time`, and `rot13`. Shakespeare wrote with less.

**README says:** _"First-class functions"_
**Reality:** Yes, actually. This part is genuinely good. We're as surprised as you are.

---

## The GitHub Stats 📈

```
Stars:       0   ██░░░░░░░░░░░░░░░░░░  (progress)
Forks:       0   ██░░░░░░░░░░░░░░░░░░  (potential)
Watchers:    0   ██░░░░░░░░░░░░░░░░░░  (stealth mode)
Issues:      0   ████████████████████  (perfection, allegedly)
PRs:         0   ████████████████████  (Saad works alone)
Contributors: 1  ████████████████████  (a legend)
```

_All zeros except the one that matters: 1 person shipped a whole programming language._

---

## Quantum vs. The Greats: A Philosophical Comparison 🧠

**Linus Torvalds** built Linux because he needed an OS.
**Saad Amin** built Quantum because he needed `rot13` in a scripting language.

Both are valid motivations. History will judge accordingly.

**Guido van Rossum** spent 4 years on Python before releasing it.
**Saad Amin** spent 3 weeks on Quantum before releasing it.

Guido had more time. Saad had more **aura**.

**Brendan Eich** built JavaScript in 10 days and the world has been suffering ever since.
**Saad Amin** built Quantum in 3 weeks and so far only one person is suffering: the person writing this README.

---

## Known Bugs 🐛

| Bug                                        | Status              | Notes                      |
| ------------------------------------------ | ------------------- | -------------------------- |
| `rot13` applied twice decrypts itself      | By Design           | This is how math works     |
| `payload` keyword does nothing             | Planned™            | Coming in v2.0.0, probably |
| Tree-walk interpreter is slow              | Won't Fix           | It has _character_         |
| 0 GitHub stars                             | Under Investigation | Possibly a GitHub bug      |
| Mixing 3 syntaxes causes existential dread | Closed              | That's a you problem       |
| `--aura` flag shows achievement board      | Working As Intended | We still won't run it      |

---

## Frequently Asked Questions (Extended Edition) ❓

**Q: Can I use Quantum in production?**
A: Define "production." If you mean "a computer that exists," technically yes.

**Q: Is Quantum faster than Python?**
A: Quantum is _more artisanal_ than Python.

**Q: Can Quantum replace my SIEM?**
A: Quantum can `rot13` your SIEM logs, which is arguably worse.

**Q: I found a bug. Where do I report it?**
A: There are 0 open issues on GitHub. Either Quantum is perfect or nobody is using it. We choose to believe both.

**Q: Why is the file extension `.sa` and not `.quantum`?**
A: Typing `quantum script.quantum` would have been too on the nose.

**Q: Is this actually a real programming language?**
A: It has a lexer, parser, AST, interpreter, REPL, OOP, closures, and a VS Code extension. It is more real than most of your side projects.

**Q: Will Quantum get more stars?**
A: You're reading this README, aren't you? Go star it. Do it. Right now.

**Q: What does SENODROOM mean?**
A: We looked into it. We found nothing. It's possible Saad is operating on a level we simply cannot comprehend.

---

## How To Contribute 🤝

1. Star the repo (you have no excuse)
2. Open an issue saying `rot13` is not real encryption
3. Saad closes the issue as "works as intended"
4. You gain respect for the process
5. Submit a PR adding `sha256`
6. It gets merged at 2am
7. You are now a core contributor to a programming language

---

## Conclusion

Look — Quantum is actually impressive. Writing a scripting language from scratch in C++17 with a lexer, parser, tree-walk interpreter, REPL, OOP, closures, AND cybersecurity builtins is genuinely a lot of work. Most people can't do this.

Saad built it in **3 weeks**.

Is it going to replace Python? No.
Is it going to replace Bash? Absolutely not.
Is `rot13` a serious cryptographic tool? I cannot stress this enough — no.
Is `SENODROOM` a real word? Unclear.
Should you star this repo? **Absolutely yes.**

But did one person build an entire programming language from scratch, document it thoroughly, write examples in three different syntax styles, build a VS Code extension, AND ship v1.0.0 in under a month?

**Yes. Yes they did.**

Mad respect. 🫡

The `--aura` was inside us all along.

---

## License

MIT — meaning you can use this to do anything, including things that would make Saad regret open-sourcing it. He won't though. He has too much aura for regret.

---

## Hall of Fame 🏅

| Achievement                                                | Recipient             |
| ---------------------------------------------------------- | --------------------- |
| Built a programming language in 3 weeks                    | Muhammad Saad Amin    |
| Shipped `rot13` as a security feature with a straight face | Muhammad Saad Amin    |
| Named a language runtime flag `--aura`                     | Muhammad Saad Amin    |
| Reserved `payload` as a keyword on a public GitHub repo    | Muhammad Saad Amin    |
| Got 0 stars and kept going                                 | Muhammad Saad Amin 🫡 |

---

_Built with love, C++17, three weeks of sleep deprivation, and an alarming disregard for the existence of Python._

**⚡ Quantum — Write once, run anywhere, hack everything (responsibly, please, we are begging you).**

> _"The only language where your encryption strategy and your cipher are the same function."_
> — This README
