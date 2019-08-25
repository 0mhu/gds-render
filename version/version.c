/*
 * GDSII-Converter
 * Copyright (C) 2018  Mario Hüttel <mario.huettel@gmx.net>
 *
 * This file is part of GDSII-Converter.
 *
 * GDSII-Converter is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * GDSII-Converter is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GDSII-Converter.  If not, see <http://www.gnu.org/licenses/>.
 */


/**
 * @defgroup version Version Number
 * See @ref git-version-num
 * @addtogroup version
 * @{
 */

#ifdef PROJECT_GIT_VERSION
#define xstr(a) str(a)
#define str(a) #a
const char *_app_version_string = xstr(PROJECT_GIT_VERSION);
#else
const char *_app_version_string = "! version not set !";
#endif

#ifdef PROJECT_GIT_COMMIT
#define xstr(a) str(a)
#define str(a) #a
const char *_app_git_commit = xstr(PROJECT_GIT_COMMIT);
#else
const char *_app_git_commit = "! Commit hash not available !";
#endif

/** @} */
