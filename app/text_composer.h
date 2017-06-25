#ifndef TEXT_COMPOSER_H
#define TEXT_COMPOSER_H


namespace tc{

        struct text_block{
        };

        struct text_item{
                virtual ~text_item()=default;
                virtual int x()const=0;
                virtual int y()const=0;
                virtual int width()const=0;
                virtual int height()const=0;
        };



} // tc

#endif // TEXT_COMPOSER_H 
