#!/bin/bash

make && gdb --args src/ss $1/model.andl $1/CTLFireability.xml