# wiki-abstracts

Hacked-together tool to build cleartext abstracts from Wikipedia articles very fast, given as a Wikipedia XML dump file. Used for the lecture Information Retrieval. This comes without any warranty.

## Usage

    $ make compile
    $ ./src/WikiAbstractsMain <WIKI XML DUMP>

Abstracts are immediately printed to stdout. Processing the entire Wikipedia takes around 30 minutes.

## Example

    Strollology      Strollology or Promenadology is the science of strolling as a method in the field of aesthetics and cultural studies with the aim of becoming aware of the conditions of perception of the environment and enhancement of environmental perception itself. Based on traditional methods in cultural studies as well as experimental practices like taking reflective walks and aesthetically interventions. The term and special field of studies was created in the 1980s by the Swiss sociologist Lucius Burckhardt, who, at that time, was a professor at the University of Kassel, as an alternative to the technocratic centrally planned economy.

## Features

* Wikitext parser which handles ``[]``, ``[[]]`` and ``{}`` and transforms them to clear text
* Normal brackets (``()``) are dropped with their content (*TODO*: make configurable)
* If a TOC is present, the abstract is the text until the TOC
* If no TOC is present, the abstract is the text until the first heading
* If no TOC and no heading is present, the abstract is the first paragraph

## TODOs

* Implement parser for ``{{as of}}`` templates
