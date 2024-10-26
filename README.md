# The Teeny, Tiny, Tat-INI Library #

## By "that rabbit," Tat🐰Rabbit ##

Experience the thrill of a ✨🪒💈close shave💈🪒✨ by entrusting
your most valuable files to the smoothest, most silky .ini reader money
can't buy!

Included is a C library for use in your own firmware/software, and a CLI tool
intended for developers/tech-savvy end-users which can be integrated into shell
scripts.

## ⚠️**WARNING: DEVELOPMENT IN PROGRESS**⚠️ ##

The documentation herein may change as the software develops. It is currently
intended to function as a **design document** until the CLI utility is mature.

## Design Philosophy ##

* 🏎️**Be Fast!** If efficiency the concern: Plaintext .ini files are read
  into memory, then mutilated in-place.
* 🐁**Be Small!** If memory is the concern, build a list of Tini Ops to
  define changes, then let the library stream reading and writing for you.
  Science never stopped to ask why we *should* parse a 3GB ini file on the
  Playdate handheld console, it only asked if it could!
* ⚡️**Be Raw!** Nothing has more flex than **fully supporting** Funny Numbers.
  Did you accidentally try to parse a "text" file with a `'\0`? No worries!
  It's a valid value!
* 👮**Be Compliant!** This mess of a "file format" is a total rabbit hole!
  `"Quotes"`? How about booleans, `NO` or `0` or `OFF`? Is a space after `=`
  part of the value, ignored, or illegal? Does it use `;` or `#` comments?
  Both? **None!?** Stop pulling your hare out. Let the funny rabbit handle it!
* 🕶️**Be Cool!** From your shell of choice, read just one value, or merge
  3 files into 1, write it to a file, merge another, and write again. Or
  make a clean, validated, and pretty-printed .ini out of nothing on the CLI.

### CLI Usage ###

`tatini [-v] options...`

**The order of CLI arguments is important!**

The main thing to grasp when understanding the CLI's usage is the fact that,
although almost every option can be specified more than once, the program is
*stateful* and each option, processed from left to right, will either change
the state (henceforth referred to as the capital S "State" for clarity), or
perform some action on—or with—the current Contents (the internal
representation of an ini file).

#### Examples ####

##### Validating a file #####

```
  # Validate in a script
$ tatini --flavor greetings.ini=validate -i hello.ini && echo "All's good!"

  # Clean a parsable, badly-formed file
$ tatini --flavor greetings.ini -i hello.ini -o hello_repaired.ini

  # Clean a parsable, badly-formed file
$ tatini --flavor greetings.ini -i hello.ini -o hello_repaired.ini

  # Change a file's flavor
$ tatini --flavor greetings.ini=validate -i hello.ini \
   -F =UseQuotes=NO -o hello_repaired.ini
```

##### Merging and saving #####

```
$ tatini --flavor foods.ini \
  -i fruits.ini -i vegetables.ini \  # Read fruits, then merge veg into it
    -o vegetarian_menu.ini \         # ...then write the state into a file
    -i meats.ini -o regular_menu.ini # Now add some meat, and write another!
```

##### Robustly look up a validated value in a script ####

```
#!/usr/bin/env bash
skv=$(tatini -F Pears=Type=?Int=validate \ # Explicitly validate an int
                -i foods.ini             \ # Parse, exit 1 if failed
                -k Fruits -g Pears=?0) # Look in the [Fruits] section for Pears)
errno=$?

if (( $errno != 0 )); then exit $errno; fi

n_pears=$(echo "$skv" | cut -f 3); # Section, Key, Value
echo "Number of pears: $n_pears"
```

#### Options ####

`--input|-i <filename>`
: Parse a file into the current State Contents, as if each key/value had
been set with `--set-value`. If `filename` is `-`, read from stdin. The file is
parsed with the state's current settings (See Parser Options, below.)

`--output|-o <filename>`
: Outputs the current State Contents to a file. This option can be specified
more than once.

`--verbose|-v`
: Verbose output.

`--flavor none|([<filename>.ini][=][silent|warn|validate])`
: Change the State's "flavor" of ini. `none` (the State's default): no
checking. Any existing Flavor rules are cleared. Files will be parsed
best-of-ability, with lax rules. This option should be avoided if the flavor in
question is known. `silent`: Files will be parsed with a specific flavor.
This enables some "extensions" sometimes seen which would be incompatible with
the way ini files are typically formatted, such as lists. It also ensures that
values (`"string"` vs. `string` vs. `number`) are treated correctly. `warn`
(the default): Like before, but outputs a warning if the file doesn't strictly
adhere to the Flavor's format specification. This is the recommended setting if
you have, or have made, a Flavor definition. `validate`: Same as before, but
additionally, immediately fail with an error code.

#### Parser Options ####

`--flavor-quirk|-F <section=key=[value[=silent|warn|validate]]`
: Set a specific value of the current State's Flavor. For scripts, It's
preferable to make a Flavor file. This option, however, is designed for an
interactive shell where robustness and forward-thinking are a waste of time.
It can, for instance, quickly set case-sensitivity and treatment of quotes.

#### Actions ####

`--set-section|-k <name>`
: Change the section currently operated on. If the State is in
`--no-fail-missing` mode, the section will be appended to the end of the
Contents, if it does not exist.

`--set-section-default|-K`
: Change the section the State is currently operating on to the default "no
section".

`--get-value|-g <name>[=?default]`
: Search the current section of the Contents for the key with the name
provided, and print its value in the current state object to stdout. This
option can be specified multiple times. Each time this appears in the command
line, an additional line will be printed. The output of each shall be the
tab-separated list `section  name  value`. In `?` mode, specify a default value
to print.

`--set-value|-s <name=[(!|?)value]>`
: Set the value in the State's current section of the Contents. If the state
is in `--no-fail-missing` mode, the value will be appended to the end of that
section if it does not exist. In `!` mode, the value is overwritten. In `?`
mode, the value is set only if empty.

## Stance on open source ##

I love open source! That is to say, I usually only take advantage of it. Far
too often, a really amazing piece of software is written, which I'd love to
use, but it's made with a license such as the GPL. I do like the MIT license,
it's great, but even the small requirement to track which libraries are used and
credit them all can be bothersome!

I believe that for a piece of free, open-source software to truly be useful, it
must be useful to *everybody* who would want to use it. Yes, that includes
corporations. Yes, that includes people who would "steal" from the project by
not giving back.

In the end, what matters is that the software is used. If it's not used, it
won't be adopted. If it's not adopted, it won't be improved. If it's not
improved, well, you get the point.

You may do anything you like with this software. I'd *appreciate* it if you
did not claim it was your own and sell it! That would be really yucky! But I
legally can't (and wouldn't want to) stop you from doing just that.

They say there's no such thing as a "free meal," and I suppose that's true.
A lot of time and energy went into whipping up this tasty dish. I hope you
enjoy!