#set($dirName = $DIR_PATH.substring($DIR_PATH.lastIndexOf("/"), $DIR_PATH.length()))
#set($dirName = $dirName.substring(1, $dirName.length()))
#if ($HEADER_COMMENTS)
#if ($ORGANIZATION_NAME && $ORGANIZATION_NAME != "")
// (C)$YEAR ${ORGANIZATION_NAME}
#else
// (C)$YEAR S3 Games
#end
// $FILE_NAME
//
#parse("hr.h")
#end