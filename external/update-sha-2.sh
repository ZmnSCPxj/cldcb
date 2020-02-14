#! /bin/sh

REPO="https://github.com/amosnier/sha-2"
EXTNAME="sha-2"
COMMIT="61555d4"

DIR=`mktemp -d`

if git clone ${REPO} "$DIR"; then
  :
else
  printf 'Failed to clone "%s".\n' "${REPO}" 1>&2
  exit 1
fi

ORIGDIR="`pwd`"

cd ${DIR}

if git reset --hard ${COMMIT}; then
  :
else
  printf 'Failed to find commit "%s".\n' "${COMMIT}" 1>&2
  cd "$ORIGDIR"
  rm -rf "${DIR}"
  exit 1
fi

find '(' -name '.git'  ')' -exec rm -rf '{}' ';'

if patch -p1 < "${ORIGDIR}"/"${EXTNAME}.patch"; then
  :
else
  printf 'Failed to apply patch "%s".\n' "${ORIGDIR}"/"${EXTNAME}.patch" 1>&2
  cd "$ORIGDIR"
  rm -rf ${DIR}
  exit 1
fi
rm -f Makefile > /dev/null 2>&1 # Because automake is better.

cd "$ORIGDIR"
rm -rf "${EXTNAME}" > /dev/null 2>&1
cp -r "${DIR}" "${EXTNAME}"
