/*
 *    Copyright (C) 2021 Joshua Boudreau <jboudreau@45drives.com>
 *    
 *    This file is part of gen-dataset.
 * 
 *    gen-dataset is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    gen-dataset is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with gen-dataset.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "getopts.hpp"
#include "gendataset.hpp"

int main(int argc, char *argv[]){
	Options opts = get_opts(argc, argv);
	
	gen_dataset(opts);
	
	return 0;
}
