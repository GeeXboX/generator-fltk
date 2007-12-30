/*
 *  Samba support for GeeXboX FLTK Generator
 *  Copyright (C) 2007 Mathieu Schroeter
 *
 *   This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *   This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *   You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef samba_h
#define samba_h

void add_smb(GeneratorUI *ui, int newadd);
void remove_smb(GeneratorUI *ui);
void add_smbshare(GeneratorUI *ui);
void remove_smbshare(GeneratorUI *ui);

void update_smb_tab(GeneratorUI *ui);
int init_samba_tab(GeneratorUI *ui);
int write_samba_settings(GeneratorUI *ui);

#endif